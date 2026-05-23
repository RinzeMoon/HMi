#include <QApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QDirIterator>
#include <QDebug>
#include <QWindow>
#include <QQmlContext>
#include <qqml.h>
#include <QQuickWindow>
#include <QOpenGLWidget>
#include <QtWebEngineQuick>
#include <QWebEngineProfile>

#include "Service/MusicService/MusicService.h"
#include "Service/TimeService/TimeService.h"
#include "Service/WeatherService/WeatherService.h"
#include "Service/LocationService/LocationService.h"
#include "Service/AudioService/AudioService.h"
#include "Service/ConfigReader/ConfigReader.h"
#include "Service/MapService/MapService.h"
#include "Service/VideoCallService/VideoCallService.h"
#include "Service/VideoCallService/VideoRenderer.h"
#include "Service/DataBus/DataBus.h"

// Agent Core
#include "Service/LLMService/LLMService.h"
#include "Service/AgentCore/AgentLoop.h"
#include "Service/AgentCore/ToolRegistry.h"
#include "Service/AgentCore/ToolExecutor.h"
#include "Service/AgentCore/MemoryManager.h"

// Tools
#include "Service/Tools/TimeTool.h"
#include "Service/Tools/WeatherTool.h"
#include "Service/Tools/MusicTool.h"
#include "Service/Tools/ClimateTool.h"
#include "Service/Tools/NavigationTool.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--ignore-gpu-blacklist --enable-gpu-rasterization");

    QtWebEngineQuick::initialize();
    QApplication app(argc, argv);

    QWebEngineProfile::defaultProfile()->clearHttpCache();
    qDebug() << "[Main] WebEngine cache cleared";

    qDebug() << "  FFmpeg configuration:";
    qDebug() << "  avcodec configuration:" << avcodec_configuration();
    qDebug() << "  avcodec version:" << avcodec_version();
    qDebug() << "  avformat version:" << avformat_version();
    qDebug() << "  avutil version:" << avutil_version();

    int solidId = QFontDatabase::addApplicationFont(":/QML/fonts/fa-solid-900.ttf");
    int regularId = QFontDatabase::addApplicationFont(":/QML/fonts/fa-regular-400.ttf");
    int brandsId = QFontDatabase::addApplicationFont(":/QML/fonts/fa-brands-400.ttf");

    qDebug() << "Solid font families:" << QFontDatabase::applicationFontFamilies(solidId);
    qDebug() << "Regular font families:" << QFontDatabase::applicationFontFamilies(regularId);
    qDebug() << "Brands font families:" << QFontDatabase::applicationFontFamilies(brandsId);

    // ── Services ──
    MusicService musicService;
    TimeService timeService;
    auto& weatherService = WeatherService::instance();
    auto& locationService = LocationService::instance();
    auto& configReader = ConfigReader::instance();
    auto& mapService = MapService::instance();
    auto& videoCallService = VideoCallService::instance();
    auto& audioService = AudioService::instance();

    // ── Agent Core ──
    auto& llmService = LLMService::instance();
    auto& memoryManager = MemoryManager::instance();

    // Tool Registry
    ToolRegistry toolRegistry;

    auto climateTool = std::make_unique<ClimateTool>();
    ClimateTool* climateToolPtr = climateTool.get();
    toolRegistry.registerTool(std::move(climateTool));

    auto navTool = std::make_unique<NavigationTool>();
    NavigationTool* navToolPtr = navTool.get();
    toolRegistry.registerTool(std::move(navTool));

    toolRegistry.registerTool(std::make_unique<TimeTool>());
    toolRegistry.registerTool(std::make_unique<WeatherTool>());
    toolRegistry.registerTool(std::make_unique<MusicTool>(&musicService));

    // Tool Executor
    ToolExecutor toolExecutor(&toolRegistry);

    // Agent Loop — the ReAct cycle engine
    AgentLoop agentLoop;
    agentLoop.setMaxSteps(6);
    agentLoop.setRegistry(&toolRegistry);
    agentLoop.setExecutor(&toolExecutor);
    agentLoop.setTools(toolRegistry.toOpenAISchema());

    qmlRegisterType<VideoRenderer>("VideoRenderer", 1, 0, "VideoRenderer");

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("musicService", &musicService);
    engine.rootContext()->setContextProperty("timeService", &timeService);
    engine.rootContext()->setContextProperty("weatherService", &weatherService);
    engine.rootContext()->setContextProperty("locationService", &locationService);
    engine.rootContext()->setContextProperty("audioService", &audioService);
    engine.rootContext()->setContextProperty("configReader", &configReader);
    engine.rootContext()->setContextProperty("mapService", &mapService);
    engine.rootContext()->setContextProperty("videoCallService", &videoCallService);

    // Agent Core — exposed to QML
    engine.rootContext()->setContextProperty("agentLoop", &agentLoop);
    engine.rootContext()->setContextProperty("memoryManager", &memoryManager);
    engine.rootContext()->setContextProperty("llmService", &llmService);

    // Data Bus — receives vehicle data from SimTool / CAN
    auto& dataBus = DataBus::instance();
    dataBus.start();
    engine.rootContext()->setContextProperty("dataBus", &dataBus);

    // Tools that emit UI signals
    engine.rootContext()->setContextProperty("climateTool", climateToolPtr);
    engine.rootContext()->setContextProperty("navTool", navToolPtr);

    qDebug() << "[Main] Registered tools:" << toolRegistry.toolNames();
    qDebug() << "[Main] All services bound to QML context";

    engine.load(QUrl("qrc:/QML/MAIN/main.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    QQuickWindow *quickWindow = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    if (quickWindow) {
        qDebug() << "[QML Window: loaded successfully]";
    }

    return app.exec();
}

#include "ConfigReader.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFile>

ConfigReader::ConfigReader(QObject *parent) : QObject(parent)
{
    // 先尝试从项目根目录读取配置文件
    QString configPath = QDir::currentPath() + "/config.ini";
    
    // 如果根目录没有，再尝试应用程序目录
    if (!QFile::exists(configPath)) {
        configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    }
    
    settings = new QSettings(configPath, QSettings::IniFormat, this);
    loadConfig();
}

ConfigReader::~ConfigReader()
{
}

ConfigReader& ConfigReader::instance()
{
    static ConfigReader instance;
    return instance;
}

void ConfigReader::loadConfig()
{
    qDebug() << "[ConfigReader] Loading config...";
    qDebug() << "[ConfigReader] Config file path:" << settings->fileName();
    
    QFile file(settings->fileName());
    if (file.exists()) {
        qDebug() << "[ConfigReader] Config file exists!";
    } else {
        qDebug() << "[ConfigReader] ERROR: Config file NOT found at:" << settings->fileName();
        qDebug() << "[ConfigReader] Current working directory:" << QDir::currentPath();
        qDebug() << "[ConfigReader] Application directory:" << QCoreApplication::applicationDirPath();
    }
    
    settings->beginGroup("DeepSeek");
    deepSeekApiKey = settings->value("api_key", "").toString();
    deepSeekModel = settings->value("model", "deepseek-chat").toString();
    deepSeekApiUrl = settings->value("api_url", "https://api.deepseek.com/chat/completions").toString();
    temperature = settings->value("temperature", 0.7).toDouble();
    maxTokens = settings->value("max_tokens", 500).toInt();
    settings->endGroup();

    settings->beginGroup("Weather");
    weatherApiKey = settings->value("api_key", "").toString();
    settings->endGroup();

    settings->beginGroup("Simulation");
    simEnabled = settings->value("enabled", true).toBool();
    simLat = settings->value("gps_latitude", 39.9087).toDouble();
    simLng = settings->value("gps_longitude", 116.3975).toDouble();
    settings->endGroup();

    settings->beginGroup("GaoDe");
    gaoDeApiKey = settings->value("api_key", "").toString();
    gaoDeJsApiKey = settings->value("js_api_key", gaoDeApiKey).toString();
    gaoDeSecurityKey = settings->value("security_key", "").toString();
    settings->endGroup();
    
    qDebug() << "[ConfigReader] DeepSeek API Key:" << (deepSeekApiKey.isEmpty() ? "Not set" : "Set (length:" + QString::number(deepSeekApiKey.length()) + ")");
    qDebug() << "[ConfigReader] DeepSeek Model:" << deepSeekModel;
    qDebug() << "[ConfigReader] DeepSeek API URL:" << deepSeekApiUrl;
    qDebug() << "[ConfigReader] Temperature:" << temperature;
    qDebug() << "[ConfigReader] Max Tokens:" << maxTokens;
    qDebug() << "[ConfigReader] GaoDe API Key:" << (gaoDeApiKey.isEmpty() ? "Not set" : "Set (length:" + QString::number(gaoDeApiKey.length()) + ")");
    qDebug() << "[ConfigReader] GaoDe JS API Key:" << (gaoDeJsApiKey.isEmpty() ? "Not set" : "Set (length:" + QString::number(gaoDeJsApiKey.length()) + ")");
    qDebug() << "[ConfigReader] GaoDe Security Key:" << (gaoDeSecurityKey.isEmpty() ? "Not set" : "Set (length:" + QString::number(gaoDeSecurityKey.length()) + ")");
}

QString ConfigReader::getDeepSeekApiKey() const
{
    return deepSeekApiKey;
}

QString ConfigReader::getDeepSeekModel() const
{
    return deepSeekModel;
}

QString ConfigReader::getDeepSeekApiUrl() const
{
    return deepSeekApiUrl;
}

double ConfigReader::getTemperature() const
{
    return temperature;
}

int ConfigReader::getMaxTokens() const
{
    return maxTokens;
}

QString ConfigReader::getWeatherApiKey() const
{
    return weatherApiKey;
}

QString ConfigReader::getGaoDeApiKey() const
{
    return gaoDeApiKey;
}

QString ConfigReader::getGaoDeJsApiKey() const
{
    return gaoDeJsApiKey;
}

QString ConfigReader::getGaoDeSecurityKey() const
{
    return gaoDeSecurityKey;
}

bool ConfigReader::isSimulationEnabled() const
{
    return simEnabled;
}

double ConfigReader::simulationLat() const
{
    return simLat;
}

double ConfigReader::simulationLng() const
{
    return simLng;
}

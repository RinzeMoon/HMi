#include "DeepSeekService.h"
#include "ConfigReader.h"
#include "NetworkManager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QMap>
#include <QDateTime>

DeepSeekService::DeepSeekService(QObject *parent) : QObject(parent)
{
}

DeepSeekService::~DeepSeekService()
{
}

DeepSeekService& DeepSeekService::instance()
{
    static DeepSeekService instance;
    return instance;
}

QJsonArray DeepSeekService::getTools()
{
    QJsonArray tools;

    QJsonObject getCurrentTimeTool;
    getCurrentTimeTool["type"] = "function";

    QJsonObject getCurrentTimeFunction;
    getCurrentTimeFunction["name"] = "get_current_time";
    getCurrentTimeFunction["description"] = "获取当前的时间和日期，不需要任何参数。当用户询问现在几点、今天几号等时间相关问题时使用此工具。";
    getCurrentTimeFunction["parameters"] = QJsonObject{
        {"type", "object"},
        {"properties", QJsonObject{}},
        {"required", QJsonArray{}}
    };
    getCurrentTimeTool["function"] = getCurrentTimeFunction;
    tools.append(getCurrentTimeTool);

    QJsonObject playMusicTool;
    playMusicTool["type"] = "function";

    QJsonObject playMusicFunction;
    playMusicFunction["name"] = "play_music";
    playMusicFunction["description"] = "播放用户指定的歌曲。song_name 参数填入歌曲名称，可以从用户的请求中提取。当用户要求播放某首歌时，直接调用此工具。";
    playMusicFunction["parameters"] = QJsonObject{
        {"type", "object"},
        {"properties", QJsonObject{
            {"song_name", QJsonObject{
                {"type", "string"},
                {"description", "要播放的歌曲名称，例如：七里香、富士山下、小幸运等"}
            }}
        }},
        {"required", QJsonArray{"song_name"}}
    };
    playMusicTool["function"] = playMusicFunction;
    tools.append(playMusicTool);

    QJsonObject getWeatherTool;
    getWeatherTool["type"] = "function";

    QJsonObject getWeatherFunction;
    getWeatherFunction["name"] = "get_weather";
    getWeatherFunction["description"] = "获取指定城市的天气信息。city 参数填入城市名称。当用户询问某个城市的天气时，直接调用此工具。";
    getWeatherFunction["parameters"] = QJsonObject{
        {"type", "object"},
        {"properties", QJsonObject{
            {"city", QJsonObject{
                {"type", "string"},
                {"description", "要查询天气的城市名称，例如：北京、上海、广州、深圳等"}
            }}
        }},
        {"required", QJsonArray{"city"}}
    };
    getWeatherTool["function"] = getWeatherFunction;
    tools.append(getWeatherTool);

    QJsonObject controlAirConditionerTool;
    controlAirConditionerTool["type"] = "function";

    QJsonObject controlAirConditionerFunction;
    controlAirConditionerFunction["name"] = "control_air_conditioner";
    controlAirConditionerFunction["description"] = "控制空调状态，包括温度设置、风扇开关和档位调节。当用户要求调节空调时，直接调用此工具。";
    controlAirConditionerFunction["parameters"] = QJsonObject{
        {"type", "object"},
        {"properties", QJsonObject{
            {"fan_active", QJsonObject{
                {"type", "boolean"},
                {"description", "风扇是否激活，true表示开启，false表示关闭"}
            }},
            {"left_temp", QJsonObject{
                {"type", "integer"},
                {"description", "左侧温度，范围16-30度"}
            }},
            {"right_temp", QJsonObject{
                {"type", "integer"},
                {"description", "右侧温度，范围16-30度"}
            }},
            {"fan_level", QJsonObject{
                {"type", "integer"},
                {"description", "风扇档位，0=自动，1-4=手动档位"}
            }}
        }},
        {"required", QJsonArray{}}
    };
    controlAirConditionerTool["function"] = controlAirConditionerFunction;
    tools.append(controlAirConditionerTool);

    QJsonObject searchNavigationTool;
    searchNavigationTool["type"] = "function";

    QJsonObject searchNavigationFunction;
    searchNavigationFunction["name"] = "search_navigation";
    searchNavigationFunction["description"] = "进行路线规划导航。当用户要求导航、去某个地方、找路时，直接调用此工具。start_address 是起点地址（可选，默认当前位置），end_address 是终点地址（必填），city 是城市名称（可选，默认北京）。";
    searchNavigationFunction["parameters"] = QJsonObject{
        {"type", "object"},
        {"properties", QJsonObject{
            {"start_address", QJsonObject{
                {"type", "string"},
                {"description", "起点地址，例如：北京西站、我的位置等，可选，不传则默认当前位置"}
            }},
            {"end_address", QJsonObject{
                {"type", "string"},
                {"description", "终点地址，例如：天安门广场、故宫博物院、清华大学等，必填"}
            }},
            {"city", QJsonObject{
                {"type", "string"},
                {"description", "城市名称，例如：北京、上海、广州等，可选，默认北京"}
            }}
        }},
        {"required", QJsonArray{"end_address"}}
    };
    searchNavigationTool["function"] = searchNavigationFunction;
    tools.append(searchNavigationTool);

    return tools;
}

void DeepSeekService::sendMessage(const QString& message)
{
    qDebug() << "[DeepSeek] Started!";
    
    ConfigReader& config = ConfigReader::instance();
    QString apiKey = config.getDeepSeekApiKey();
    QString apiUrl = config.getDeepSeekApiUrl();
    QString model = config.getDeepSeekModel();
    double temperature = config.getTemperature();
    int maxTokens = config.getMaxTokens();

    qDebug() << "[DeepSeek] Config - API URL:" << apiUrl;
    qDebug() << "[DeepSeek] Config - Model:" << model;
    qDebug() << "[DeepSeek] Config - API Key:" << (apiKey.isEmpty() ? "Not set" : "Set (length:" + QString::number(apiKey.length()) + ")");

    if (apiKey.isEmpty()) {
        qDebug() << "[DeepSeek] ERROR: API Key not configured!";
        emit errorOccurred("API Key not configured! Please set api_key in config.ini");
        return;
    }

    qDebug() << "[DeepSeek] Sending message:" << message;
    emit typingStarted();

    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = message;
    conversationHistory.append(userMessage);

    QJsonArray messagesArray;

    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "你是一个乐于助人的AI助手。请务必遵循以下规则：\n1. 当用户询问当前时间或日期时，必须调用 get_current_time 工具\n2. 当用户要求播放音乐或歌曲时，必须调用 play_music 工具，song_name 参数填入歌曲名称\n3. 当用户询问天气时，必须调用 get_weather 工具，city 参数填入城市名称\n4. 当用户要求调节空调时，必须调用 control_air_conditioner 工具，可以设置 fan_active（风扇开关）、left_temp（左侧温度）、right_temp（右侧温度）、fan_level（风扇档位）参数\n5. 当用户要求导航、去某个地方、找路、规划路线时，必须调用 search_navigation 工具，end_address 参数填入终点地址，start_address 可选（默认为当前位置），city 可选（默认北京）\n6. 不要编造信息，必须使用工具获取真实数据\n7. 如果用户提到了歌曲名、城市名、目的地地址或空调设置，直接提取并调用相应工具即可";
    messagesArray.append(systemMessage);

    for (const auto& msg : conversationHistory) {
        messagesArray.append(msg);
    }

    sendMessageWithTools(messagesArray);
}

void DeepSeekService::sendMessageWithTools(const QJsonArray& messages)
{
    ConfigReader& config = ConfigReader::instance();
    QString apiKey = config.getDeepSeekApiKey();
    QString apiUrl = config.getDeepSeekApiUrl();
    QString model = config.getDeepSeekModel();
    double temperature = config.getTemperature();
    int maxTokens = config.getMaxTokens();

    QJsonObject json;
    json["model"] = model;
    json["temperature"] = temperature;
    json["max_tokens"] = maxTokens;
    json["messages"] = messages;
    json["tools"] = getTools();
    json["tool_choice"] = "auto";

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    qDebug() << "[DeepSeek] Tools being sent:" << getTools();
    qDebug() << "[DeepSeek] Request payload:" << jsonData;

    QMap<QByteArray, QByteArray> headers;
    headers["Authorization"] = QString("Bearer %1").arg(apiKey).toUtf8();
    
    qDebug() << "[DeepSeek] Sending POST request to:" << apiUrl;
    
    NetworkManager& nm = NetworkManager::instance();
    QNetworkReply *reply = nm.postWithHeaders(QUrl(apiUrl), jsonData, headers);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyReceived(reply);
    });
}

void DeepSeekService::onReplyReceived(QNetworkReply* reply)
{
    qDebug() << "[DeepSeek] Received reply from server";
    
    if (!reply) {
        qDebug() << "[DeepSeek] ERROR: Reply is null!";
        emit typingFinished();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[DeepSeek] ERROR: Network error:" << reply->errorString();
        emit errorOccurred("Network error: " + reply->errorString());
        emit typingFinished();
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    qDebug() << "[DeepSeek] Raw data:" << responseData;

    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject()) {
        qDebug() << "[DeepSeek] ERROR: Invalid JSON response";
        emit errorOccurred("Invalid response from server");
        emit typingFinished();
        reply->deleteLater();
        return;
    }

    QJsonObject json = doc.object();
    qDebug() << "[DeepSeek] Parsed JSON successfully";

    if (json.contains("error")) {
        QJsonObject errorObj = json["error"].toObject();
        QString errorMessage = errorObj["message"].toString("Unknown error");
        qDebug() << "[DeepSeek] ERROR: API error:" << errorMessage;
        emit errorOccurred("API error: " + errorMessage);
        emit typingFinished();
        reply->deleteLater();
        return;
    }

    QJsonArray choices = json["choices"].toArray();
    if (choices.isEmpty()) {
        qDebug() << "[DeepSeek] ERROR: No choices in response";
        emit errorOccurred("No response from AI");
        emit typingFinished();
        reply->deleteLater();
        return;
    }

    qDebug() << "[DeepSeek] Success! Got" << choices.size() << "choice(s)";

    QJsonObject firstChoice = choices.first().toObject();
    QJsonObject messageObj = firstChoice["message"].toObject();

    conversationHistory.append(messageObj);

    if (messageObj.contains("tool_calls")) {
        qDebug() << "[DeepSeek] Received tool call request";
        QJsonArray toolCalls = messageObj["tool_calls"].toArray();
        
        for (const auto& tc : toolCalls) {
            QJsonObject toolCallObj = tc.toObject();
            QString toolCallId = toolCallObj["id"].toString();
            QJsonObject functionObj = toolCallObj["function"].toObject();
            QString name = functionObj["name"].toString();
            QString args = functionObj["arguments"].toString();
            
            qDebug() << "[DeepSeek] Tool:" << name << "Args:" << args;
            
            pendingToolCallId = toolCallId;
            emit toolCallRequested(name, args);
        }
    } else {
        QString content = messageObj["content"].toString();
        qDebug() << "[DeepSeek] AI Response:" << content;
        emit messageReceived("assistant", content);
        emit typingFinished();
        qDebug() << "[DeepSeek] Completed!";
    }

    reply->deleteLater();
}

void DeepSeekService::submitToolResult(const QString& result)
{
    qDebug() << "[DeepSeek] Submitting tool result:" << result;
    
    if (pendingToolCallId.isEmpty()) {
        qDebug() << "[DeepSeek] ERROR: No pending tool call!";
        return;
    }

    QJsonObject toolResultMessage;
    toolResultMessage["role"] = "tool";
    toolResultMessage["tool_call_id"] = pendingToolCallId;
    toolResultMessage["content"] = result;
    conversationHistory.append(toolResultMessage);

    pendingToolCallId.clear();

    QJsonArray messagesArray;

    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "你是一个乐于助人的AI助手。请务必遵循以下规则：\n1. 当用户询问当前时间或日期时，必须调用 get_current_time 工具\n2. 当用户要求播放音乐或歌曲时，必须调用 play_music 工具，song_name 参数填入歌曲名称\n3. 当用户询问天气时，必须调用 get_weather 工具，city 参数填入城市名称\n4. 当用户要求调节空调时，必须调用 control_air_conditioner 工具，可以设置 fan_active（风扇开关）、left_temp（左侧温度）、right_temp（右侧温度）、fan_level（风扇档位）参数\n5. 当用户要求导航、去某个地方、找路、规划路线时，必须调用 search_navigation 工具，end_address 参数填入终点地址，start_address 可选（默认为当前位置），city 可选（默认北京）\n6. 不要编造信息，必须使用工具获取真实数据\n7. 如果用户提到了歌曲名、城市名、目的地地址或空调设置，直接提取并调用相应工具即可";
    messagesArray.append(systemMessage);

    for (const auto& msg : conversationHistory) {
        messagesArray.append(msg);
    }

    sendMessageWithTools(messagesArray);
}

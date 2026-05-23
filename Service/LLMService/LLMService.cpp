#include "LLMService.h"
#include "ConfigReader.h"
#include "NetworkManager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

LLMService::LLMService(QObject *parent) : QObject(parent)
{
    m_systemPrompt = QStringLiteral(
        "你是一个车载智能助手。请务必遵守以下规则：\n"
        "1. 当用户询问当前时间或日期时，必须调用 get_current_time 工具\n"
        "2. 当用户要求播放音乐或歌曲时，必须调用 play_music 工具\n"
        "3. 当用户询问天气时，必须调用 get_weather 工具\n"
        "4. 当用户要求调节空调时，必须调用 control_air_conditioner 工具\n"
        "5. 当用户要求导航、规划路线时，必须调用 search_navigation 工具\n"
        "6. 不要编造信息，必须使用工具获取真实数据\n"
        "7. 如果用户提到了歌曲名、城市名、目的地地址或空调设置，直接提取并调用相应工具"
    );
}

LLMService::~LLMService() {}

LLMService& LLMService::instance()
{
    static LLMService instance;
    return instance;
}

void LLMService::cancel()
{
    m_streamingActive = false;
    m_streamBuffer.clear();
    m_currentContent.clear();
}

void LLMService::sendMessage(const QString& message)
{
    qDebug() << "[LLMService] Sending message:" << message;

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = message;
    m_conversationHistory.append(userMsg);
}

void LLMService::sendMessageWithContext(const QJsonArray& messages,
                                         const QJsonArray& tools)
{
    QJsonArray fullMessages;
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = m_systemPrompt;
    fullMessages.append(systemMsg);

    for (int i = 0; i < messages.size(); ++i) {
        fullMessages.append(messages[i]);
    }

    chatWithTools(fullMessages, tools);
}

void LLMService::chatWithTools(const QJsonArray& messages, const QJsonArray& tools)
{
    ConfigReader& config = ConfigReader::instance();
    QString apiKey = config.getDeepSeekApiKey();
    QString apiUrl = config.getDeepSeekApiUrl();
    QString model = config.getDeepSeekModel();
    double temperature = config.getTemperature();
    int maxTokens = config.getMaxTokens();

    if (apiKey.isEmpty()) {
        emit errorOccurred("API Key not configured");
        return;
    }

    QJsonObject json;
    json["model"] = model;
    json["temperature"] = temperature;
    json["max_tokens"] = maxTokens;
    json["messages"] = messages;
    json["stream"] = true;
    if (!tools.isEmpty()) {
        json["tools"] = tools;
        json["tool_choice"] = "auto";
    }

    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    qDebug() << "[LLMService] Request (streaming):" << jsonData.left(300);

    m_streamBuffer.clear();
    m_currentContent.clear();
    m_currentReasoning.clear();
    m_responseBody.clear();
    m_accumulatedToolCalls.clear();
    m_streamingActive = true;

    QMap<QByteArray, QByteArray> headers;
    headers["Authorization"] = QString("Bearer %1").arg(apiKey).toUtf8();

    NetworkManager& nm = NetworkManager::instance();
    QNetworkReply* reply = nm.postWithHeaders(QUrl(apiUrl), jsonData, headers);
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        if (!m_streamingActive) {
            reply->abort();
            return;
        }
        QByteArray data = reply->readAll();
        m_responseBody.append(data);
        processStreamChunk(data);
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyReceived(reply);
    });
}

void LLMService::processStreamChunk(const QByteArray& chunk)
{
    m_streamBuffer.append(chunk);

    while (true) {
        int idx = m_streamBuffer.indexOf('\n');
        if (idx < 0) break;

        QByteArray line = m_streamBuffer.left(idx).trimmed();
        m_streamBuffer.remove(0, idx + 1);

        if (line.isEmpty()) continue;
        if (line == "data: [DONE]") {
            m_streamingActive = false;
            continue;
        }
        if (!line.startsWith("data: ")) continue;

        QByteArray jsonData = line.mid(6);  // strip "data: "
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);
        if (err.error != QJsonParseError::NoError) continue;

        QJsonObject obj = doc.object();
        QJsonArray choices = obj["choices"].toArray();
        if (choices.isEmpty()) continue;

        QJsonObject choice = choices.first().toObject();
        QJsonObject delta = choice["delta"].toObject();

        if (delta.contains("reasoning_content")) {
            m_currentReasoning += delta["reasoning_content"].toString();
        }

        if (delta.contains("content")) {
            QString text = delta["content"].toString();
            m_currentContent += text;
            emit streamingDelta(text);
        }

        if (delta.contains("tool_calls")) {
            QJsonArray toolCalls = delta["tool_calls"].toArray();
            for (int i = 0; i < toolCalls.size(); ++i) {
                QJsonObject tcObj = toolCalls[i].toObject();
                int idx = tcObj.contains("index") ? tcObj["index"].toInt() : i;

                ToolCallAccum& accum = m_accumulatedToolCalls[idx];

                if (tcObj.contains("id") && !tcObj["id"].toString().isEmpty())
                    accum.id = tcObj["id"].toString();

                QJsonObject func = tcObj["function"].toObject();
                if (func.contains("name") && !func["name"].toString().isEmpty())
                    accum.name = func["name"].toString();
                if (func.contains("arguments"))
                    accum.arguments += func["arguments"].toString();
            }
        }
    }
}

void LLMService::onReplyReceived(QNetworkReply* reply)
{
    m_streamingActive = false;

    if (!reply) {
        emit chatFinished();
        return;
    }

    // Read any remaining data
    QByteArray remaining = reply->readAll();
    if (!remaining.isEmpty()) {
        m_responseBody.append(remaining);
        processStreamChunk(remaining);
    }

    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "[LLMService] HTTP status:" << httpStatus;

    if (reply->error() != QNetworkReply::NoError || (httpStatus > 0 && httpStatus != 200)) {
        // Try to extract error message from response body
        QString errMsg;
        QJsonDocument doc = QJsonDocument::fromJson(m_responseBody);
        if (doc.isObject() && doc.object().contains("error")) {
            QJsonObject errObj = doc.object()["error"].toObject();
            errMsg = errObj["message"].toString("Unknown API error");
        } else if (!m_responseBody.isEmpty()) {
            errMsg = QString::fromUtf8(m_responseBody.left(500));
        } else {
            errMsg = reply->errorString();
        }
        qDebug() << "[LLMService] Error - HTTP:" << httpStatus
                 << "Qt error:" << reply->errorString()
                 << "Body:" << m_responseBody;
        emit errorOccurred(QString("[HTTP %1] %2").arg(httpStatus).arg(errMsg));
        reply->deleteLater();
        return;
    }

    flushAccumulatedToolCalls();

    if (!m_currentContent.isEmpty()) {
        QJsonObject assistantMsg;
        assistantMsg["role"] = "assistant";
        assistantMsg["content"] = m_currentContent;
        m_conversationHistory.append(assistantMsg);
        emit messageComplete(m_currentContent);
    }

    emit chatFinished();
    reply->deleteLater();
}

void LLMService::flushAccumulatedToolCalls()
{
    for (auto it = m_accumulatedToolCalls.begin(); it != m_accumulatedToolCalls.end(); ++it) {
        const ToolCallAccum& accum = it.value();
        if (accum.name.isEmpty()) continue;

        QJsonObject argsObj;
        if (!accum.arguments.isEmpty()) {
            QJsonDocument adoc = QJsonDocument::fromJson(accum.arguments.toUtf8());
            if (adoc.isObject()) argsObj = adoc.object();
        }

        qDebug() << "[LLMService] Flushing tool call:" << accum.name
                 << "id:" << accum.id << "args:" << accum.arguments;
        emit toolCallRequested(accum.name, argsObj, accum.id);
    }
}

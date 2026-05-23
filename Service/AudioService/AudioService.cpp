#include "AudioService.h"
#include <QAudioFormat>
#include <QAudioSource>
#include <QAudioDevice>
#include <QDebug>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QHash>
#include "NetworkManager.h"

AudioService::AudioService(QObject *parent) : QObject(parent)
{
    audioSource = nullptr;
    buffer = nullptr;
    recordingThread = nullptr;
}

AudioService::~AudioService()
{
    if (recordingThread) {
        recordingThread->quit();
        recordingThread->wait();
        delete recordingThread;
    }
}

AudioService& AudioService::instance()
{
    static AudioService instance;
    return instance;
}

void AudioService::startRecording()
{
    if (audioSource) return;

    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    const QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (inputDevice.isNull()) {
        qWarning() << "No default audio input device available!";
        return;
    }
    if (!inputDevice.isFormatSupported(format)) {
        qWarning() << "Default format not supported - falling back to device's preferred format";
        format = inputDevice.preferredFormat();
    }

    buffer = new QBuffer();
    buffer->open(QIODevice::WriteOnly);

    audioSource = new QAudioSource(inputDevice, format, this);
    audioSource->start(buffer);

    qDebug() << "Recording started, buffer instance address:" << buffer << ", current data pointer:" << (buffer->data().isEmpty() ? "null" : buffer->data().data());
}

void AudioService::stopRecording()
{
    if (!audioSource) return;

    audioSource->stop();
    delete audioSource;
    audioSource = nullptr;

    qDebug() << "Recording stopped, buffer size:" << buffer->data().size() << "bytes";
    
    m_audioData = buffer->data();
    
    checkAudioFormatInMemory();
    
    AudioFormatInfo info = getAudioFormatInfo();
    if (info.isCompatible) {
        qDebug() << "Audio format compatible, sending to Whisper.cpp...";
        sendAudioToWhisper();
    } else {
        qDebug() << "Audio format not compatible, skipping upload";
    }

    delete buffer;
    buffer = nullptr;
}

AudioService::AudioFormatInfo AudioService::getAudioFormatInfo() {
    AudioFormatInfo info;
    info.isCompatible = false;
    info.sampleRate = 0;
    info.channels = 0;
    info.bitDepth = 0;
    info.duration = 0.0;
    
    if (!buffer || buffer->data().isEmpty()) {
        qDebug() << "Audio buffer is empty";
        return info;
    }
    
    const QByteArray& data = buffer->data();
    qDebug() << "Buffer data size:" << data.size();
    
    info.sampleRate = 16000;
    info.channels = 1;
    info.bitDepth = 16;
    
    int headerSize = 44;
    qint64 dataSize = data.size() - headerSize;
    if (dataSize > 0) {
        int bytesPerSample = info.channels * (info.bitDepth / 8);
        qint64 sampleCount = dataSize / bytesPerSample;
        info.duration = sampleCount / (double)info.sampleRate;
    }
    
    info.isCompatible = (info.sampleRate == 16000 && info.channels == 1 && info.bitDepth == 16);
    
    return info;
}

void AudioService::checkAudioFormatInMemory() {
    AudioFormatInfo info = getAudioFormatInfo();
    
    qDebug() << "========================================";
    qDebug() << "       Audio Format Debug";
    qDebug() << "========================================";
    qDebug() << "Sample Rate:" << info.sampleRate << "Hz";
    qDebug() << "Channels:" << info.channels;
    qDebug() << "Bit Depth:" << info.bitDepth << "bit";
    qDebug() << "Duration:" << info.duration << "s";
    qDebug() << "Compatible with Whisper.cpp:" << (info.isCompatible ? "YES" : "NO");
    
    if (!info.isCompatible) {
        qDebug() << "----------------------------------------";
        qDebug() << "       Compatibility Issues";
        qDebug() << "----------------------------------------";
        if (info.sampleRate != 16000) qDebug() << "✗ Sample rate should be 16000Hz";
        if (info.channels != 1) qDebug() << "✗ Should be mono (1 channel)";
        if (info.bitDepth != 16) qDebug() << "✗ Bit depth should be 16-bit";
    }
    qDebug() << "========================================";
}

void AudioService::sendAudioToWhisper() {
    if (m_audioData.isEmpty()) {
        qDebug() << "No audio data to send";
        return;
    }
    
    QByteArray wavData;
    
    QDataStream stream(&wavData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    int sampleRate = 16000;
    int channels = 1;
    int bitDepth = 16;
    int byteRate = sampleRate * channels * (bitDepth / 8);
    int blockAlign = channels * (bitDepth / 8);
    int dataSize = m_audioData.size();
    int chunkSize = 36 + dataSize;
    int fileSize = 4 + 8 + chunkSize;
    
    stream.writeRawData("RIFF", 4);
    stream << (quint32)chunkSize;
    stream.writeRawData("WAVE", 4);
    stream.writeRawData("fmt ", 4);
    stream << (quint32)16;
    stream << (quint16)1;
    stream << (quint16)channels;
    stream << (quint32)sampleRate;
    stream << (quint32)byteRate;
    stream << (quint16)blockAlign;
    stream << (quint16)bitDepth;
    stream.writeRawData("data", 4);
    stream << (quint32)dataSize;
    
    wavData.append(m_audioData);
    
    QNetworkRequest request(QUrl("http://localhost:8080/inference"));
    
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"recording.wav\""));
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/wav"));
    audioPart.setBody(wavData);
    
    QHttpPart languagePart;
    languagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"language\""));
    languagePart.setBody("zh-CN");
    
    QHttpPart responseFormatPart;
    responseFormatPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"response_format\""));
    responseFormatPart.setBody("json");
    
    multiPart->append(audioPart);
    multiPart->append(languagePart);
    multiPart->append(responseFormatPart);
    
    QNetworkReply *reply = NetworkManager::instance().post(QUrl("http://localhost:8080/inference"), multiPart);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onTranscriptionReceived();
        reply->deleteLater();
    });
    
    qDebug() << "Sending audio to Whisper.cpp at http://localhost:8080/inference";
}

void AudioService::onTranscriptionReceived() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    qDebug() << "Received response from Whisper.cpp:" << responseData;
    
    QByteArray cleanData = responseData;
    
    if (cleanData.startsWith('"') && cleanData.endsWith('"')) {
        cleanData = cleanData.mid(1, cleanData.size() - 2);
        cleanData.replace("\\\"", "\"");
        cleanData.replace("\\n", "\n");
        cleanData.replace("\\r", "\r");
        cleanData.replace("\\t", "\t");
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(cleanData);
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON response, trying original data";
        doc = QJsonDocument::fromJson(responseData);
        if (!doc.isObject()) {
            qDebug() << "Still invalid JSON response";
            return;
        }
    }
    
    QJsonObject json = doc.object();
    QString transcription = json["text"].toString();
    
    if (transcription.isEmpty()) {
        transcription = json["transcription"].toString();
    }
    
    if (transcription.isEmpty()) {
        QJsonArray segments = json["segments"].toArray();
        if (!segments.isEmpty()) {
            QJsonObject firstSegment = segments.first().toObject();
            transcription = firstSegment["text"].toString();
        }
    }
    
    qDebug() << "Transcription result:" << transcription;
    
    QString simplifiedText = traditionalToSimplified(transcription);
    qDebug() << "Simplified result:" << simplifiedText;
    
    if (!simplifiedText.isEmpty()) {
        emit transcriptionResult(simplifiedText);
    }
}

QString AudioService::traditionalToSimplified(const QString& text) {
    QString result = text;
    
    result.replace(QChar(0x8A9E), QChar(0x8BED));
    result.replace(QChar(0x8F49), QChar(0x8F6C));
    result.replace(QChar(0x6E2C), QChar(0x6D4B));
    result.replace(QChar(0x8A66), QChar(0x8BD5));
    result.replace(QChar(0x8AAA), QChar(0x8BF4));
    result.replace(QChar(0x8B1A), QChar(0x8BD7));
    result.replace(QChar(0x70BA), QChar(0x4E3A));
    result.replace(QChar(0x6642), QChar(0x65F6));
    result.replace(QChar(0x958B), QChar(0x95EE));
    result.replace(QChar(0x898B), QChar(0x89C1));
    result.replace(QChar(0x8ECA), QChar(0x5F53));
    result.replace(QChar(0x8F09), QChar(0x8F66));
    result.replace(QChar(0x9023), QChar(0x8FDE));
    result.replace(QChar(0x8FDB), QChar(0x8FDB));
    result.replace(QChar(0x9084), QChar(0x8FD8));
    result.replace(QChar(0x9060), QChar(0x9065));
    result.replace(QChar(0x9577), QChar(0x957F));
    result.replace(QChar(0x9580), QChar(0x95E8));
    result.replace(QChar(0x8A00), QChar(0x4E07));
    result.replace(QChar(0x7121), QChar(0x706F));
    result.replace(QChar(0x6F22), QChar(0x6C49));
    
    return result;
}

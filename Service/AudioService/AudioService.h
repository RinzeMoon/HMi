#ifndef AUDIOSERVICE_H
#define AUDIOSERVICE_H

#include <QObject>
#include <QAudioSource>
#include <QBuffer>
#include <QThread>
#include <QAudioDevice>
#include <QMediaDevices>

class AudioService : public QObject
{
    Q_OBJECT
public:
    static AudioService& instance();
    ~AudioService();

    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();

signals:
    void transcriptionResult(const QString& text);

private:
    explicit AudioService(QObject *parent = nullptr);
    AudioService(const AudioService&) = delete;
    AudioService& operator=(const AudioService&) = delete;

    struct AudioFormatInfo {
        int sampleRate;
        int channels;
        int bitDepth;
        double duration;
        bool isCompatible;
    };
    
    AudioFormatInfo getAudioFormatInfo();
    void checkAudioFormatInMemory();
    void sendAudioToWhisper();
    void onTranscriptionReceived();
    QString traditionalToSimplified(const QString& text);

    QAudioSource *audioSource;
    QBuffer *buffer;
    QThread *recordingThread;
    QByteArray m_audioData;
};

#endif // AUDIOSERVICE_H
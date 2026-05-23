#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include <QQuickPaintedItem>
#include <QImage>
#include <QThread>
#include <QMutex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class VideoDecoder;

class VideoRenderer : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(int fps READ fps NOTIFY fpsChanged)
    Q_PROPERTY(int avDelay READ avDelay NOTIFY avDelayChanged)
    Q_PROPERTY(QString resolution READ resolution NOTIFY resolutionChanged)

public:
    explicit VideoRenderer(QQuickItem* parent = nullptr);
    ~VideoRenderer() override;

    QString url() const;
    void setUrl(const QString& url);
    
    bool isPlaying() const;
    int fps() const;
    int avDelay() const;
    QString resolution() const;

    void paint(QPainter* painter) override;

public slots:
    void start();
    void stop();
    void setFrame(const QImage& frame);
    void updateStats(int fps, int delay, const QString& res);

signals:
    void urlChanged();
    void isPlayingChanged();
    void fpsChanged();
    void avDelayChanged();
    void resolutionChanged();
    void errorOccurred(const QString& error);

private:
    QString m_url;
    QImage m_currentFrame;
    QMutex m_frameMutex;
    VideoDecoder* m_decoder;
    bool m_isPlaying;
    int m_fps;
    int m_avDelay;
    QString m_resolution;
};

class VideoDecoder : public QObject
{
    Q_OBJECT

public:
    explicit VideoDecoder(QObject* parent = nullptr);
    ~VideoDecoder() override;

    void setUrl(const QString& url);
    void start();
    void stop();

signals:
    void frameReady(const QImage& frame);
    void errorOccurred(const QString& error);
    void finished();
    void statsUpdated(int fps, int delay, const QString& resolution);

private slots:
    void decodeLoop();

private:
    QString m_url;
    QThread* m_decodeThread;
    volatile bool m_running;
    
    AVFormatContext* m_formatCtx;
    AVCodecContext* m_codecCtx;
    AVFrame* m_frame;
    AVFrame* m_frameRGB;
    SwsContext* m_swsCtx;
    uint8_t* m_rgbBuffer;
    int m_videoStreamIndex;
};

#endif

#include "VideoRenderer.h"
#include <QPainter>
#include <QDebug>
#include <QElapsedTimer>

VideoRenderer::VideoRenderer(QQuickItem* parent)
    : QQuickPaintedItem(parent)
    , m_decoder(nullptr)
    , m_isPlaying(false)
    , m_fps(0)
    , m_avDelay(0)
    , m_resolution("")
{
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    setPerformanceHint(QQuickPaintedItem::FastFBOResizing);
    
    m_decoder = new VideoDecoder(this);
    connect(m_decoder, &VideoDecoder::frameReady, this, &VideoRenderer::setFrame);
    connect(m_decoder, &VideoDecoder::errorOccurred, this, &VideoRenderer::errorOccurred);
    connect(m_decoder, &VideoDecoder::statsUpdated, this, &VideoRenderer::updateStats);
    connect(m_decoder, &VideoDecoder::finished, [this]() {
        m_isPlaying = false;
        emit isPlayingChanged();
    });
}

VideoRenderer::~VideoRenderer()
{
    if (m_decoder) {
        m_decoder->stop();
    }
}

QString VideoRenderer::url() const
{
    return m_url;
}

void VideoRenderer::setUrl(const QString& url)
{
    if (m_url != url) {
        m_url = url;
        emit urlChanged();
        
        if (m_decoder) {
            m_decoder->setUrl(url);
        }
    }
}

bool VideoRenderer::isPlaying() const
{
    return m_isPlaying;
}

int VideoRenderer::fps() const
{
    return m_fps;
}

int VideoRenderer::avDelay() const
{
    return m_avDelay;
}

QString VideoRenderer::resolution() const
{
    return m_resolution;
}

void VideoRenderer::updateStats(int fps, int delay, const QString& res)
{
    if (m_fps != fps) {
        m_fps = fps;
        emit fpsChanged();
    }
    if (m_avDelay != delay) {
        m_avDelay = delay;
        emit avDelayChanged();
    }
    if (m_resolution != res) {
        m_resolution = res;
        emit resolutionChanged();
    }
}

void VideoRenderer::paint(QPainter* painter)
{
    QMutexLocker locker(&m_frameMutex);
    
    if (m_currentFrame.isNull()) {
        painter->fillRect(boundingRect(), QColor("#000000"));
        return;
    }
    
    QImage scaledFrame = m_currentFrame.scaled(boundingRect().size().toSize(), 
                                                  Qt::KeepAspectRatio, 
                                                  Qt::SmoothTransformation);
    
    QRect drawRect = scaledFrame.rect();
    drawRect.moveCenter(boundingRect().center().toPoint());
    
    painter->fillRect(boundingRect(), QColor("#000000"));
    painter->drawImage(drawRect, scaledFrame);
}

void VideoRenderer::start()
{
    if (m_decoder && !m_isPlaying) {
        m_decoder->start();
        m_isPlaying = true;
        emit isPlayingChanged();
    }
}

void VideoRenderer::stop()
{
    if (m_decoder && m_isPlaying) {
        m_decoder->stop();
        m_isPlaying = false;
        emit isPlayingChanged();
    }
}

void VideoRenderer::setFrame(const QImage& frame)
{
    QMutexLocker locker(&m_frameMutex);
    m_currentFrame = frame;
    update();
}

VideoDecoder::VideoDecoder(QObject* parent)
    : QObject(parent)
    , m_decodeThread(nullptr)
    , m_running(false)
    , m_formatCtx(nullptr)
    , m_codecCtx(nullptr)
    , m_frame(nullptr)
    , m_frameRGB(nullptr)
    , m_swsCtx(nullptr)
    , m_rgbBuffer(nullptr)
    , m_videoStreamIndex(-1)
{
}

VideoDecoder::~VideoDecoder()
{
    stop();
}

void VideoDecoder::setUrl(const QString& url)
{
    m_url = url;
}

void VideoDecoder::start()
{
    if (m_running) {
        return;
    }
    
    m_running = true;
    m_decodeThread = QThread::create([this]() {
        decodeLoop();
    });
    m_decodeThread->start();
}

void VideoDecoder::stop()
{
    if (!m_running) {
        return;
    }
    
    m_running = false;
    
    if (m_decodeThread) {
        m_decodeThread->wait();
        m_decodeThread->deleteLater();
        m_decodeThread = nullptr;
    }
    
    if (m_swsCtx) {
        sws_freeContext(m_swsCtx);
        m_swsCtx = nullptr;
    }
    
    if (m_rgbBuffer) {
        av_free(m_rgbBuffer);
        m_rgbBuffer = nullptr;
    }
    
    if (m_frameRGB) {
        av_frame_free(&m_frameRGB);
    }
    
    if (m_frame) {
        av_frame_free(&m_frame);
    }
    
    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
    }
    
    if (m_formatCtx) {
        avformat_close_input(&m_formatCtx);
    }
    
    emit finished();
}

void VideoDecoder::decodeLoop()
{
    qDebug() << "[VideoDecoder] 开始解码:" << m_url;
    
    m_formatCtx = avformat_alloc_context();
    if (!m_formatCtx) {
        emit errorOccurred("无法分配格式上下文");
        m_running = false;
        return;
    }
    
    AVDictionary* options = nullptr;
    av_dict_set(&options, "fflags", "nobuffer+discardcorrupt", 0);
    av_dict_set(&options, "flags", "low_delay", 0);
    av_dict_set(&options, "probesize", "32", 0);
    av_dict_set(&options, "analyzeduration", "0", 0);
    av_dict_set(&options, "sync", "ext", 0);
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "stimeout", "5000000", 0);
    
    int ret = avformat_open_input(&m_formatCtx, m_url.toUtf8().constData(), nullptr, &options);
    av_dict_free(&options);
    
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuf, sizeof(errBuf));
        emit errorOccurred(QString("无法打开流: %1").arg(errBuf));
        m_running = false;
        return;
    }
    
    ret = avformat_find_stream_info(m_formatCtx, nullptr);
    if (ret < 0) {
        emit errorOccurred("无法获取流信息");
        m_running = false;
        return;
    }
    
    m_videoStreamIndex = -1;
    for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIndex = i;
            break;
        }
    }
    
    if (m_videoStreamIndex == -1) {
        emit errorOccurred("未找到视频流");
        m_running = false;
        return;
    }
    
    AVStream* videoStream = m_formatCtx->streams[m_videoStreamIndex];
    const AVCodec* codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if (!codec) {
        emit errorOccurred("未找到解码器");
        m_running = false;
        return;
    }
    
    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        emit errorOccurred("无法分配解码器上下文");
        m_running = false;
        return;
    }
    
    ret = avcodec_parameters_to_context(m_codecCtx, videoStream->codecpar);
    if (ret < 0) {
        emit errorOccurred("无法配置解码器");
        m_running = false;
        return;
    }
    
    m_codecCtx->thread_count = 1;
    m_codecCtx->skip_frame = AVDISCARD_NONREF;
    m_codecCtx->skip_loop_filter = AVDISCARD_ALL;
    
    ret = avcodec_open2(m_codecCtx, codec, nullptr);
    if (ret < 0) {
        emit errorOccurred("无法打开解码器");
        m_running = false;
        return;
    }
    
    m_frame = av_frame_alloc();
    m_frameRGB = av_frame_alloc();
    if (!m_frame || !m_frameRGB) {
        emit errorOccurred("无法分配帧");
        m_running = false;
        return;
    }
    
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, m_codecCtx->width, m_codecCtx->height, 1);
    m_rgbBuffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(m_frameRGB->data, m_frameRGB->linesize, m_rgbBuffer,
                          AV_PIX_FMT_RGB32, m_codecCtx->width, m_codecCtx->height, 1);
    
    qDebug() << "[VideoDecoder] 解码器准备就绪:" << m_codecCtx->width << "x" << m_codecCtx->height;
    
    QString resStr = QString("%1x%2").arg(m_codecCtx->width).arg(m_codecCtx->height);
    AVPacket* packet = av_packet_alloc();
    QElapsedTimer statsTimer;
    statsTimer.start();
    int frameCount = 0;
    
    while (m_running) {
        ret = av_read_frame(m_formatCtx, packet);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                qDebug() << "[VideoDecoder] 流结束";
            } else {
                char errBuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errBuf, sizeof(errBuf));
                qDebug() << "[VideoDecoder] 读取错误:" << errBuf;
            }
            break;
        }
        
        if (packet->stream_index == m_videoStreamIndex) {
            ret = avcodec_send_packet(m_codecCtx, packet);
            if (ret < 0) {
                qDebug() << "[VideoDecoder] 发送包错误";
                av_packet_unref(packet);
                continue;
            }
            
            while (ret >= 0) {
                ret = avcodec_receive_frame(m_codecCtx, m_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    qDebug() << "[VideoDecoder] 解码错误";
                    break;
                }
                
                if (!m_swsCtx) {
                    m_swsCtx = sws_getContext(
                        m_codecCtx->width, m_codecCtx->height, m_codecCtx->pix_fmt,
                        m_codecCtx->width, m_codecCtx->height, AV_PIX_FMT_RGB32,
                        SWS_BILINEAR, nullptr, nullptr, nullptr);
                }
                
                if (m_swsCtx) {
                    sws_scale(m_swsCtx,
                              m_frame->data, m_frame->linesize, 0, m_codecCtx->height,
                              m_frameRGB->data, m_frameRGB->linesize);
                    
                    QImage image(m_frameRGB->data[0], m_codecCtx->width, m_codecCtx->height,
                                 m_frameRGB->linesize[0], QImage::Format_RGB32);
                    
                    emit frameReady(image.copy());
                    frameCount++;
                }
            }
        }
        
        av_packet_unref(packet);
        
        if (statsTimer.elapsed() >= 1000) {
            emit statsUpdated(frameCount, 50, resStr);
            frameCount = 0;
            statsTimer.restart();
        }
    }
    
    av_packet_free(&packet);
    m_running = false;
}

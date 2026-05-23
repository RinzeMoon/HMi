#ifndef MUSICSERVICE_H
#define MUSICSERVICE_H

#include "../RateLimitedService/RateLimitedService.h"
#include "../../NetworkManager/NetworkManager.h"
#include <QObject>
#include <QVariantList>

// 音质等级枚举
enum class SongQuality {
    Standard,   // 标准
    Higher,     // 较高
    ExHigh,     // 极高
    Lossless    // 无损
};

class MusicService : public RateLimitedService
{
    Q_OBJECT
    Q_PROPERTY(QString baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged)

public:
    explicit MusicService(QObject *parent = nullptr);

    QString baseUrl() const;
    void setBaseUrl(const QString &url);

    // 搜索歌曲（分页）
    Q_INVOKABLE void searchSongs(const QString &keywords, int limit = 30, int offset = 0);

    // 获取歌单详情（包含所有歌曲）
    Q_INVOKABLE void getPlaylistDetail(int playlistId, int limit = 1000, int offset = 0);

    // 获取歌曲播放 URL（可指定音质）
    Q_INVOKABLE void getSongUrl(int songId, SongQuality quality = SongQuality::Standard);

    // 获取歌词
    Q_INVOKABLE void getLyric(int songId);

    // 获取歌曲详情（包括封面）
    Q_INVOKABLE void getSongDetail(int songId);

    signals:
        void baseUrlChanged();
    void searchResult(const QVariantList &songs, bool hasMore);
    void playlistDetailResult(int playlistId, const QVariantList &songs, bool hasMore);
    void songUrlResult(int songId, const QString &url);
    void lyricResult(int songId, const QString &lyric);
    void songDetailResult(int songId, const QVariantMap &songDetail);
    void errorOccurred(const QString &message, const QString &details = QString());

private:
    // 处理响应的私有函数（非槽，通过 lambda 调用）
    void handleSearchReply(QNetworkReply *reply);
    void handlePlaylistDetailReply(QNetworkReply *reply, int playlistId);
    void handleSongUrlReply(QNetworkReply *reply, int songId);
    void handleLyricReply(QNetworkReply *reply, int songId);
    void handleSongDetailReply(QNetworkReply *reply, int songId);

    // 工具函数
    QString qualityToString(SongQuality quality) const;
    bool handleCommonError(QNetworkReply *reply, QJsonDocument &doc, QJsonObject &obj, QByteArray *rawData = nullptr);

    QString m_baseUrl = "http://localhost:3000";
};

#endif // MUSICSERVICE_H
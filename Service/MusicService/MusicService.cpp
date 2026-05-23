#include "MusicService.h"
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MusicService::MusicService(QObject *parent)
    : RateLimitedService(2, parent) // 默认每秒2次
{
    qDebug() << "[MusicService] 构造完成，baseUrl =" << m_baseUrl;
}

QString MusicService::baseUrl() const { return m_baseUrl; }
void MusicService::setBaseUrl(const QString &url)
{
    if (m_baseUrl != url) {
        m_baseUrl = url;
        emit baseUrlChanged();
        qDebug() << "[MusicService] baseUrl 已修改为" << m_baseUrl;
    }
}

QString MusicService::qualityToString(SongQuality quality) const
{
    QString str;
    switch (quality) {
    case SongQuality::Standard: str = "standard"; break;
    case SongQuality::Higher:   str = "higher"; break;
    case SongQuality::ExHigh:   str = "exhigh"; break;
    case SongQuality::Lossless: str = "lossless"; break;
    default: str = "standard";
    }
    qDebug() << "[MusicService] 音质枚举转换:" << int(quality) << "->" << str;
    return str;
}

bool MusicService::handleCommonError(QNetworkReply *reply, QJsonDocument &doc, QJsonObject &obj, QByteArray *rawData)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[MusicService] 网络错误:" << reply->errorString();
        emit errorOccurred("网络错误", reply->errorString());
        return false;
    }

    QByteArray data = reply->readAll();
    qDebug() << "[MusicService] 收到原始数据长度:" << data.size();
    
    if (rawData) {
        *rawData = data;
    }

    QJsonParseError err;
    doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qDebug() << "[MusicService] JSON解析错误:" << err.errorString();
        emit errorOccurred("JSON解析错误", err.errorString());
        return false;
    }

    obj = doc.object();
    int code = obj["code"].toInt();
    qDebug() << "[MusicService] 响应状态码:" << code;

    if (code != 200) {
        QString msg = obj["msg"].toString();
        if (msg.isEmpty()) msg = obj["message"].toString();
        qDebug() << "[MusicService] API错误:" << code << msg;
        emit errorOccurred("API错误", QString("Code: %1, Msg: %2").arg(code).arg(msg));
        return false;
    }
    return true;
}

// ---------- 搜索 ----------
void MusicService::searchSongs(const QString &keywords, int limit, int offset)
{
    qDebug() << "[MusicService] 搜索歌曲: 关键词=" << keywords << ", limit=" << limit << ", offset=" << offset;

    QUrl url(m_baseUrl + "/search");
    QUrlQuery query;
    query.addQueryItem("keywords", keywords);
    query.addQueryItem("limit", QString::number(limit));
    query.addQueryItem("offset", QString::number(offset));
    query.addQueryItem("type", "1");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtCarHMI/1.0");

    enqueueRequest([this, request]() {
        qDebug() << "[MusicService] 发送搜索请求";
        QNetworkReply *reply = NetworkManager::instance().sendRequest(request, "GET");
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            handleSearchReply(reply);
        });
    });
}

void MusicService::handleSearchReply(QNetworkReply *reply)
{
    qDebug() << "[MusicService] 收到搜索响应";

    QJsonDocument doc;
    QJsonObject obj;
    if (!handleCommonError(reply, doc, obj)) {
        reply->deleteLater();
        return;
    }

    QJsonObject result = obj["result"].toObject();
    QJsonArray songs = result["songs"].toArray();
    bool hasMore = result["hasMore"].toBool();
    QVariantList songList;

    qDebug() << "[MusicService] 搜索结果数量:" << songs.size();

    for (const auto &songVal : songs) {
        QJsonObject s = songVal.toObject();
        QVariantMap item;
        item["id"] = s["id"].toInt();
        item["name"] = s["name"].toString();

        QString artist;
        QJsonArray artists = s["artists"].toArray();
        if (artists.isEmpty()) artists = s["ar"].toArray();
        for (int i = 0; i < artists.size(); ++i) {
            if (i > 0) artist += " / ";
            artist += artists[i].toObject()["name"].toString();
        }
        item["artist"] = artist;

        QJsonObject album = s["album"].toObject();
        if (album.isEmpty()) album = s["al"].toObject();
        item["album"] = album["name"].toString();
        item["albumPic"] = album["picUrl"].toString();
        item["duration"] = s["duration"].toInt();
        if (item["duration"].isNull()) item["duration"] = s["dt"].toInt();

        songList << item;
    }

    if (!songList.isEmpty()) {
        qDebug() << "[MusicService] 第一条歌曲数据:" << songList.first().toMap()["name"].toString()
                 << "-" << songList.first().toMap()["artist"].toString();
    }

    emit searchResult(songList, hasMore);
    reply->deleteLater();
}

// ---------- 歌单详情 ----------
void MusicService::getPlaylistDetail(int playlistId, int limit, int offset)
{
    qDebug() << "[MusicService] 获取歌单详情: playlistId=" << playlistId << ", limit=" << limit << ", offset=" << offset;

    QUrl url(m_baseUrl + "/playlist/track/all");
    QUrlQuery query;
    query.addQueryItem("id", QString::number(playlistId));
    query.addQueryItem("limit", QString::number(limit));
    query.addQueryItem("offset", QString::number(offset));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtCarHMI/1.0");

    enqueueRequest([this, request, playlistId]() {
        qDebug() << "[MusicService] 发送歌单详情请求";
        QNetworkReply *reply = NetworkManager::instance().sendRequest(request, "GET");
        connect(reply, &QNetworkReply::finished, this, [this, reply, playlistId]() {
            handlePlaylistDetailReply(reply, playlistId);
        });
    });
}

void MusicService::handlePlaylistDetailReply(QNetworkReply *reply, int playlistId)
{
    qDebug() << "[MusicService] 收到歌单详情响应, playlistId=" << playlistId;

    QJsonDocument doc;
    QJsonObject obj;
    if (!handleCommonError(reply, doc, obj)) {
        reply->deleteLater();
        return;
    }

    QJsonArray songs = obj["songs"].toArray();
    bool hasMore = obj["hasMore"].toBool();
    QVariantList songList;

    qDebug() << "[MusicService] 歌单歌曲数量:" << songs.size();

    for (const auto &songVal : songs) {
        QJsonObject s = songVal.toObject();
        QVariantMap item;
        item["id"] = s["id"].toInt();
        item["name"] = s["name"].toString();

        QString artist;
        QJsonArray artists = s["ar"].toArray();
        for (int i = 0; i < artists.size(); ++i) {
            if (i > 0) artist += " / ";
            artist += artists[i].toObject()["name"].toString();
        }
        item["artist"] = artist;

        QJsonObject album = s["al"].toObject();
        item["album"] = album["name"].toString();
        item["albumPic"] = album["picUrl"].toString();
        item["duration"] = s["dt"].toInt();

        songList << item;
    }

    if (!songList.isEmpty()) {
        qDebug() << "[MusicService] 歌单第一条歌曲:" << songList.first().toMap()["name"].toString();
    }

    emit playlistDetailResult(playlistId, songList, hasMore);
    reply->deleteLater();
}

// ---------- 歌曲URL ----------
void MusicService::getSongUrl(int songId, SongQuality quality)
{
    qDebug() << "[MusicService] 获取歌曲URL: songId=" << songId << ", quality=" << qualityToString(quality);

    QUrl url(m_baseUrl + "/song/url/v1");
    QUrlQuery query;
    query.addQueryItem("id", QString::number(songId));
    query.addQueryItem("level", qualityToString(quality));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtCarHMI/1.0");

    enqueueRequest([this, request, songId]() {
        qDebug() << "[MusicService] 发送歌曲URL请求";
        QNetworkReply *reply = NetworkManager::instance().sendRequest(request, "GET");
        connect(reply, &QNetworkReply::finished, this, [this, reply, songId]() {
            handleSongUrlReply(reply, songId);
        });
    });
}

void MusicService::handleSongUrlReply(QNetworkReply *reply, int songId)
{
    qDebug() << "[MusicService] 收到歌曲URL响应, songId=" << songId;

    QJsonDocument doc;
    QJsonObject obj;
    if (!handleCommonError(reply, doc, obj)) {
        reply->deleteLater();
        return;
    }

    QJsonArray dataArr = obj["data"].toArray();
    if (dataArr.isEmpty()) {
        qDebug() << "[MusicService] 歌曲URL数据为空, songId=" << songId;
        emit songUrlResult(songId, "");
        reply->deleteLater();
        return;
    }

    QString url = dataArr.first().toObject()["url"].toString();
    qDebug() << "[MusicService] 歌曲URL结果:" << url;
    emit songUrlResult(songId, url);
    reply->deleteLater();
}

// ---------- 歌词 ----------
void MusicService::getLyric(int songId)
{
    qDebug() << "[MusicService] 获取歌词: songId=" << songId;

    QUrl url(m_baseUrl + "/lyric");
    QUrlQuery query;
    query.addQueryItem("id", QString::number(songId));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtCarHMI/1.0");

    enqueueRequest([this, request, songId]() {
        qDebug() << "[MusicService] 发送歌词请求";
        QNetworkReply *reply = NetworkManager::instance().sendRequest(request, "GET");
        connect(reply, &QNetworkReply::finished, this, [this, reply, songId]() {
            handleLyricReply(reply, songId);
        });
    });
}

void MusicService::handleLyricReply(QNetworkReply *reply, int songId)
{
    qDebug() << "[MusicService] 收到歌词响应, songId=" << songId;

    QJsonDocument doc;
    QJsonObject obj;
    if (!handleCommonError(reply, doc, obj)) {
        reply->deleteLater();
        return;
    }

    QString lyric;
    if (obj.contains("lrc")) {
        lyric = obj["lrc"].toObject()["lyric"].toString();
        qDebug() << "[MusicService] 歌词长度:" << lyric.length() << "字符";
    } else {
        qDebug() << "[MusicService] 响应中无歌词字段";
    }

    emit lyricResult(songId, lyric);
    reply->deleteLater();
}

// ---------- 歌曲详情 ----------
void MusicService::getSongDetail(int songId)
{
    qDebug() << "[MusicService] 获取歌曲详情: songId=" << songId;

    QUrl url(m_baseUrl + "/song/detail");
    QUrlQuery query;
    query.addQueryItem("ids", QString::number(songId));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtCarHMI/1.0");

    enqueueRequest([this, request, songId]() {
        qDebug() << "[MusicService] 发送歌曲详情请求";
        QNetworkReply *reply = NetworkManager::instance().sendRequest(request, "GET");
        connect(reply, &QNetworkReply::finished, this, [this, reply, songId]() {
            handleSongDetailReply(reply, songId);
        });
    });
}

void MusicService::handleSongDetailReply(QNetworkReply *reply, int songId)
{
    qDebug() << "[MusicService] 收到歌曲详情响应, songId=" << songId;

    QJsonDocument doc;
    QJsonObject obj;
    QByteArray rawData;
    if (!handleCommonError(reply, doc, obj, &rawData)) {
        reply->deleteLater();
        return;
    }

    // 调试：打印原始数据
    qDebug() << "[MusicService] 原始响应数据:" << QString::fromUtf8(rawData);
    
    obj = doc.object();
    int code = obj["code"].toInt();
    qDebug() << "[MusicService] 响应状态码:" << code;

    if (code != 200) {
        QString msg = obj["msg"].toString();
        if (msg.isEmpty()) msg = obj["message"].toString();
        qDebug() << "[MusicService] API错误:" << code << msg;
        emit errorOccurred("API错误", QString("Code: %1, Msg: %2").arg(code).arg(msg));
        reply->deleteLater();
        return;
    }

    QJsonArray songs = obj["songs"].toArray();
    if (songs.isEmpty()) {
        qDebug() << "[MusicService] 歌曲详情数据为空, songId=" << songId;
        emit songDetailResult(songId, QVariantMap());
        reply->deleteLater();
        return;
    }

    QJsonObject song = songs.first().toObject();
    QVariantMap detail;
    detail["id"] = song["id"].toInt();
    detail["name"] = song["name"].toString();
    
    QJsonArray artists = song["ar"].toArray();
    QString artist;
    for (int i = 0; i < artists.size(); ++i) {
        if (i > 0) artist += " / ";
        artist += artists[i].toObject()["name"].toString();
    }
    detail["artist"] = artist;
    
    QJsonObject album = song["al"].toObject();
    detail["album"] = album["name"].toString();
    detail["albumPic"] = album["picUrl"].toString();
    detail["duration"] = song["dt"].toInt();
    detail["copyrightId"] = song["cp"].toInt();
    
    qDebug() << "[MusicService] 歌曲详情结果: name=" << detail["name"].toString() << ", albumPic=" << detail["albumPic"].toString();
    emit songDetailResult(songId, detail);
    reply->deleteLater();
}
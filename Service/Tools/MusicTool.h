#ifndef MUSICTOOL_H
#define MUSICTOOL_H

#include "Service/AgentCore/AgentTool.h"
#include "Service/MusicService/MusicService.h"

#include <QFuture>
#include <QPromise>
#include <QEventLoop>
#include <QJsonDocument>

class MusicTool : public AgentTool
{
    Q_OBJECT
public:
    explicit MusicTool(MusicService* svc, QObject *parent = nullptr)
        : AgentTool(parent), m_svc(svc) {}

    QString name() const override { return "play_music"; }
    QString description() const override
    {
        return QStringLiteral(
            "播放用户指定的歌曲。song_name 参数填入歌曲名称，可以从用户的请求中提取。"
            "当用户要求播放某首歌时，直接调用此工具。");
    }
    QJsonObject parametersSchema() const override
    {
        return QJsonObject{
            {"type", "object"},
            {"properties", QJsonObject{
                {"song_name", QJsonObject{
                    {"type", "string"},
                    {"description", "要播放的歌曲名称，例如：七里香、富士山下、小幸运等"}
                }}
            }},
            {"required", QJsonArray{"song_name"}}
        };
    }

    QFuture<QString> execute(const QJsonObject& args) override
    {
        QPromise<QString> promise;
        promise.start();

        QString songName = args["song_name"].toString();
        if (songName.isEmpty()) {
            promise.addResult("Error: song_name is required");
            promise.finish();
            return promise.future();
        }

        m_svc->searchSongs(songName, 1, 0);

        QEventLoop loop;
        QMetaObject::Connection c1, c2;

        c1 = connect(m_svc, &MusicService::searchResult,
            this, [&](const QVariantList& songs, bool) {
                disconnect(c1);
                disconnect(c2);
                if (!songs.isEmpty()) {
                    QVariantMap firstSong = songs.first().toMap();
                    QString name = firstSong["name"].toString();
                    int songId = firstSong["id"].toInt();
                    m_svc->getSongUrl(songId, SongQuality::Standard);
                    promise.addResult(QString("已成功搜索并开始播放：%1").arg(name));
                } else {
                    promise.addResult(QString("未找到歌曲：%1").arg(songName));
                }
                promise.finish();
                loop.quit();
            });

        c2 = connect(m_svc, &MusicService::errorOccurred,
            this, [&](const QString& msg, const QString&) {
                disconnect(c1);
                disconnect(c2);
                promise.addResult(QString("音乐搜索失败：%1").arg(msg));
                promise.finish();
                loop.quit();
            });

        loop.exec();
        return promise.future();
    }

private:
    MusicService* m_svc;
};

#endif // MUSICTOOL_H

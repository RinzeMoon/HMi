//
// Created by admin on 2026/3/16.
//

#ifndef WEATHERSERVICE_H
#define WEATHERSERVICE_H

#include <QObject>
#include <QNetworkReply>

class WeatherService : public QObject
{
    Q_OBJECT
public:
    static WeatherService& instance();

    // 设置API密钥
    Q_INVOKABLE void setApiKey(const QString &apiKey);

    // 通过城市名称获取天气
    Q_INVOKABLE void getWeatherByCity(const QString &city);

    // 通过经纬度获取天气
    Q_INVOKABLE void getWeatherByCoordinates(double latitude, double longitude);

    // 获取天气图标代码
    Q_INVOKABLE QString getWeatherIcon(const QString &weatherCode);

    // 设置是否使用模拟数据
    Q_INVOKABLE void setUseMockData(bool useMock);

signals:
    // 天气数据获取成功
    void weatherDataReceived(const QString &location, int temperature, const QString &condition, const QString &icon);

    // 天气数据获取失败
    void weatherDataError(const QString &error);

private slots:
    void onWeatherReplyFinished(QNetworkReply *reply);

private:
    explicit WeatherService(QObject *parent = nullptr);
    ~WeatherService();

    QString m_apiKey;
    static const QString BASE_URL;
    bool m_useMockData;
};

#endif // WEATHERSERVICE_H

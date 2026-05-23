//
// Created by admin on 2026/3/16.
//

#include "WeatherService.h"
#include "../../NetworkManager/NetworkManager.h"
#include "ConfigReader.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QTimer>
#include <QRandomGenerator>

const QString WeatherService::BASE_URL = "https://api.openweathermap.org/data/2.5/weather";

WeatherService::WeatherService(QObject *parent) : QObject(parent)
{
    m_apiKey = ConfigReader::instance().getWeatherApiKey();
    m_useMockData = ConfigReader::instance().isSimulationEnabled();
}

WeatherService::~WeatherService()
{}

WeatherService &WeatherService::instance()
{
    static WeatherService instance;
    return instance;
}

void WeatherService::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

void WeatherService::setUseMockData(bool useMock)
{
    m_useMockData = useMock;
}

void WeatherService::getWeatherByCity(const QString &city)
{
    if (m_useMockData) {
        // 使用模拟数据
        QTimer::singleShot(100, this, [this, city]() {
            QStringList conditions = {"晴", "多云", "阴", "小雨", "晴间多云"};
            QStringList weatherCodes = {"01d", "03d", "04d", "10d", "02d"};
            int randomIndex = QRandomGenerator::global()->bounded(conditions.size());
            int temperature = 15 + QRandomGenerator::global()->bounded(15);
            
            emit weatherDataReceived(city, temperature, conditions[randomIndex], getWeatherIcon(weatherCodes[randomIndex]));
        });
        return;
    }
    
    QUrl url(BASE_URL);
    QUrlQuery query;
    query.addQueryItem("q", city);
    query.addQueryItem("appid", m_apiKey);
    query.addQueryItem("units", "metric"); // 使用摄氏度
    query.addQueryItem("lang", "zh_cn"); // 使用中文
    url.setQuery(query);

    QNetworkReply *reply = NetworkManager::instance().get(url);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onWeatherReplyFinished(reply);
    });
}

void WeatherService::getWeatherByCoordinates(double latitude, double longitude)
{
    if (m_useMockData) {
        // 使用模拟数据
        QTimer::singleShot(100, this, [this, latitude, longitude]() {
            QStringList conditions = {"晴", "多云", "阴", "小雨", "晴间多云"};
            QStringList weatherCodes = {"01d", "03d", "04d", "10d", "02d"};
            QStringList locations = {"北京市", "上海市", "广州市", "深圳市", "杭州市"};
            int randomIndex = QRandomGenerator::global()->bounded(conditions.size());
            int tempIndex = QRandomGenerator::global()->bounded(locations.size());
            int temperature = 15 + QRandomGenerator::global()->bounded(15);
            
            emit weatherDataReceived(locations[tempIndex], temperature, conditions[randomIndex], getWeatherIcon(weatherCodes[randomIndex]));
        });
        return;
    }
    
    QUrl url(BASE_URL);
    QUrlQuery query;
    query.addQueryItem("lat", QString::number(latitude));
    query.addQueryItem("lon", QString::number(longitude));
    query.addQueryItem("appid", m_apiKey);
    query.addQueryItem("units", "metric"); // 使用摄氏度
    query.addQueryItem("lang", "zh_cn"); // 使用中文
    url.setQuery(query);

    QNetworkReply *reply = NetworkManager::instance().get(url);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onWeatherReplyFinished(reply);
    });
}

QString WeatherService::getWeatherIcon(const QString &weatherCode)
{
    // 映射天气代码到Font Awesome图标
    if (weatherCode.startsWith("01")) {
        return "\uf185"; // 晴天
    } else if (weatherCode.startsWith("02")) {
        return "\uf0c2"; // 少云
    } else if (weatherCode.startsWith("03") || weatherCode.startsWith("04")) {
        return "\uf0c2"; // 多云
    } else if (weatherCode.startsWith("09") || weatherCode.startsWith("10")) {
        return "\uf076"; // 雨
    } else if (weatherCode.startsWith("11")) {
        return "\uf0e7"; // 雷雨
    } else if (weatherCode.startsWith("13")) {
        return "\uf2dc"; // 雪
    } else if (weatherCode.startsWith("50")) {
        return "\uf74a"; // 雾
    }
    return "\uf185"; // 默认晴天
}

void WeatherService::onWeatherReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit weatherDataError(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObj = jsonDoc.object();

    if (jsonObj.contains("name") && jsonObj.contains("main") && jsonObj.contains("weather")) {
        QString location = jsonObj["name"].toString();
        int temperature = jsonObj["main"].toObject()["temp"].toInt();
        QJsonArray weatherArray = jsonObj["weather"].toArray();
        if (!weatherArray.isEmpty()) {
            QJsonObject weatherObj = weatherArray[0].toObject();
            QString condition = weatherObj["description"].toString();
            QString weatherCode = weatherObj["icon"].toString();
            QString icon = getWeatherIcon(weatherCode);

            emit weatherDataReceived(location, temperature, condition, icon);
        }
    } else {
        emit weatherDataError("Invalid weather data");
    }

    reply->deleteLater();
}

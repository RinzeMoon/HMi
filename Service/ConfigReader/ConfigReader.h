#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <QObject>
#include <QString>
#include <QSettings>

class ConfigReader : public QObject
{
    Q_OBJECT
public:
    static ConfigReader& instance();
    ~ConfigReader();

    Q_INVOKABLE QString getDeepSeekApiKey() const;
    Q_INVOKABLE QString getDeepSeekModel() const;
    Q_INVOKABLE QString getDeepSeekApiUrl() const;
    Q_INVOKABLE double getTemperature() const;
    Q_INVOKABLE int getMaxTokens() const;
    Q_INVOKABLE QString getGaoDeApiKey() const;
    Q_INVOKABLE QString getGaoDeJsApiKey() const;
    Q_INVOKABLE QString getGaoDeSecurityKey() const;
    Q_INVOKABLE QString getWeatherApiKey() const;
    Q_INVOKABLE bool isSimulationEnabled() const;
    Q_INVOKABLE double simulationLat() const;
    Q_INVOKABLE double simulationLng() const;

signals:
    void configChanged();

private:
    explicit ConfigReader(QObject *parent = nullptr);
    ConfigReader(const ConfigReader&) = delete;
    ConfigReader& operator=(const ConfigReader&) = delete;

    void loadConfig();

    QSettings *settings;
    QString deepSeekApiKey;
    QString deepSeekModel;
    QString deepSeekApiUrl;
    double temperature;
    int maxTokens;
    QString gaoDeApiKey;
    QString gaoDeJsApiKey;
    QString gaoDeSecurityKey;
    QString weatherApiKey;
    bool simEnabled = true;
    double simLat = 39.9087;
    double simLng = 116.3975;
};

#endif // CONFIGREADER_H

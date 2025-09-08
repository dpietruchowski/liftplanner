#pragma once
#include <QObject>
#include <QColor>
#include <QString>
#include <QMap>

class ColoredSvgProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString svgSource READ svgSource NOTIFY svgSourceChanged)
    Q_PROPERTY(QString svgOriginSource READ svgOriginSource WRITE setSvgOriginSource NOTIFY svgOriginSourceChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)

public:
    explicit ColoredSvgProvider(QObject *parent = nullptr);

    QString svgSource() const { return m_svgSource; }
    QString svgOriginSource() const { return m_svgOriginSource; }
    void setSvgOriginSource(const QString &s);

    QColor color() const { return m_color; }
    void setColor(const QColor &c);

    QString icon() const { return m_icon; }
    void setIcon(const QString &s);

signals:
    void svgSourceChanged();
    void svgOriginSourceChanged();
    void colorChanged();
    void iconChanged();

private:
    void updateSvgSource();
    QString loadSvgContent(const QString &path);

    QString m_svgSource;
    QString m_svgOriginSource;
    QString m_icon;
    QColor m_color;
    static QMap<QString, QString> s_svgCache;
};
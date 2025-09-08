#include "coloredsvgprovider.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QUrl>

QMap<QString, QString> ColoredSvgProvider::s_svgCache;

ColoredSvgProvider::ColoredSvgProvider(QObject *parent) : QObject(parent) {}

void ColoredSvgProvider::setSvgOriginSource(const QString &s) {
    if (m_svgOriginSource == s) return;
    m_svgOriginSource = s;
    emit svgOriginSourceChanged();
    updateSvgSource();
}

void ColoredSvgProvider::setIcon(const QString &s) {
    if (m_icon == s) return;
    m_icon = s;
    emit iconChanged();
    updateSvgSource();
}

void ColoredSvgProvider::setColor(const QColor &c) {
    if (m_color == c) return;
    m_color = c;
    emit colorChanged();
    updateSvgSource();
}

QString ColoredSvgProvider::loadSvgContent(const QString &path) {
    if (s_svgCache.contains(path))
        return s_svgCache.value(path);

    QString realPath = path;
    if (realPath.startsWith("qrc:"))
        realPath.replace("qrc:", ":");

    QFile file(realPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open SVG:" << realPath;
        return QString();
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    s_svgCache.insert(path, content);
    return content;
}

void ColoredSvgProvider::updateSvgSource() {
    QString path = !m_svgOriginSource.isEmpty() ? m_svgOriginSource : m_icon;
    if (path.isEmpty()) {
        m_svgSource.clear();
        emit svgSourceChanged();
        return;
    }

    QString svg = loadSvgContent(path);
    if (svg.isEmpty()) {
        m_svgSource.clear();
        emit svgSourceChanged();
        return;
    }

    if (m_color.isValid()) {
        QString colorStr = m_color.name(QColor::HexRgb);
        svg.replace("currentColor", colorStr, Qt::CaseInsensitive);
    }

    m_svgSource = "data:image/svg+xml;utf8," + svg.toUtf8().toPercentEncoding();
    emit svgSourceChanged();
}
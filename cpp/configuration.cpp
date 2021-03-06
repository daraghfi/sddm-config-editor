#include "configuration.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QSettings>
#include <QDebug>
#include "section.h"
#include "controller.h"

Configuration::Configuration(QObject* parent) : QObject(parent),
  m_sections()
{
  m_controller = parent;
}

void Configuration::loadSchema(const QJsonArray& schema)
{
  for(QJsonArray::const_iterator element = schema.begin();
      element != schema.end(); element++) {
    if(!element->isObject()) {
      qDebug() << "Malformed configuration schema";
    } else {
      Section *section = Section::fromJson(element->toObject(), this);
      m_sections.append(section);
    }
  }
}

void Configuration::loadSettings(QSettings& qsettings)
{
  foreach(QObject* section, m_sections) {
    QString name = section->property("name").toString();
    QList<QObject*> settings = reinterpret_cast<Section*>(section)->property("settings")
                              .value<QList<QObject*> >();
    foreach(QObject* setting, settings) {
      QString key = setting->property("key").toString();
      QVariant value = qsettings.value(QString("%1/%2").arg(name, key),
                                       QVariant::fromValue(QString("")));
      reinterpret_cast<Setting*>(setting)->setProperty("value", value);
    }
  }
  emit sectionsChanged();
}

QList<QObject*> Configuration::model()
{
  return m_sections;
}

QString Configuration::toFile() const
{
  QString file = "# Generated by SDDM Configuration Editor\n";
  foreach(QObject* section, m_sections) {
    file += reinterpret_cast<Section*>(section)->toString();
  }
  return file;
}

QDebug operator<<(QDebug debug, const Configuration &configuration)
{
  debug.nospace();
  foreach(QObject* section, configuration.m_sections) {
    debug << *reinterpret_cast<Section*>(section);
  }

  return debug.space();
}


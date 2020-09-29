#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "model/FilterModel.h"

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <cmath>

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    static ProjectManager& instance()
    {
        static ProjectManager instance;
        return instance;
    }

    ProjectManager(ProjectManager const&) = delete;
    void operator=(ProjectManager const&) = delete;

    void initialize(FilterModel* model){
        m_model = model;
    };

    bool saveProject(const QString& fileName);
    bool exportProject(QString fileName, const std::list<double>& p1, const std::list<double>& p2);
    bool loadProject(const QString& fileName);
    void closeProject();

    bool loadParametricEq(const QString& fileName);
    bool loadParametricEqString(QString string);
    bool loadVdc(const QString &fileName);

    static bool writeProject(const QString& fileName, QVector<Biquad*> bank);
    static bool writeProject(const QString &fileName, QVector<DeflatedBiquad> bank);
    static QVector<DeflatedBiquad> readProject(const QString& fileName);
    static DeflatedBiquad parseProjectLine(QString str);
    static QVector<DeflatedBiquad> readParametricEq(const QString &path);
    static DeflatedBiquad parseParametricEqLine(QString string);
    static QVector<DeflatedBiquad> readVdc(const QString &inputVdc);

    QString currentProject() const
    {
        return m_currentProject;
    }

    bool hasUnsavedChanges() const
    {
        return m_projectUnsaved;
    }

public slots:
    void projectModified();

signals:
    void projectClosed();
    void projectMetaChanged();

private:
    ProjectManager();
    FilterModel* m_model;
    QString m_currentProject;
    bool m_projectUnsaved;
};

#endif // PROJECTMANAGER_H

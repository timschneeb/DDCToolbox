#ifndef AUTOEQCLIENT_H
#define AUTOEQCLIENT_H

#include "loghelper.h"

#include <QDebug>
#include <QObject>
#include <QPixmap>

#ifndef IS_WASM
#include <NetworkRequest.h>
#endif

#define ROOT_RAW_URL "https://github.com/jaakkopasanen/AutoEq/raw/master"
#define RESULTS_RAW_URL ROOT_RAW_URL"/results"
#define INDEX_RAW_URL RESULTS_RAW_URL"/INDEX.md"

#define ROOT_API_URL "https://api.github.com/repos/jaakkopasanen/AutoEq"
#define CONTENTS_API_URL ROOT_API_URL"/contents"

class QueryResult;
class QueryRequest;
class HeadphoneMeasurement;

class AutoEQClient
{
public:
    static QVector<QueryResult> query(QueryRequest request);
    static HeadphoneMeasurement fetchDetails(QueryResult item);
};

class QueryRequest
{
public:
    QueryRequest(QString modelFilter = "", QString groupFilter = ""){
        mModelFilter = modelFilter;
        mGroupFilter = groupFilter;
    }

    const QString getModelFilter(){
        return mModelFilter;
    }
    const QString getGroupFilter(){
        return mGroupFilter;
    }
    bool isModelFilterEnabled(){
        return !mModelFilter.isEmpty();
    }
    bool isGroupFilterEnabled(){
        return !mGroupFilter.isEmpty();
    }

    QString mModelFilter;
    QString mGroupFilter;
};

class QueryResult
{
public:
    QueryResult(QString model = "", QString group = "", QString apipath = ""){
        mModel = model;
        mGroup = group;
        mApiPath = apipath;
    }

    const QString getModel(){
        return mModel;
    }
    const QString getGroup(){
        return mGroup;
    }
    const QString getAPIPath(){
        return mApiPath;
    }

    QString mModel;
    QString mGroup;
    QString mApiPath;
};

class HeadphoneMeasurement
{
public:
    HeadphoneMeasurement(QString model = "", QString group = "",
                         QString parameq = "", QString graphurl = ""){
        mModel = model;
        mGroup = group;
        mParamEQ = parameq;
        mGraphUrl = graphurl;
    }

    const QString getModel(){
        return mModel;
    }
    const QString getGroup(){
        return mGroup;
    }
    const QString getParametricEQ(){
        return mParamEQ;
    }
    const QString getGraphUrl(){
        return mGraphUrl;
    }
    void setModel(QString model){
        mModel = model;
    }
    void setGroup(QString group){
        mGroup = group;
    }
    void setParametricEQ(QString parameq){
        mParamEQ = parameq;
    }
    void setGraphUrl(QString graphurl){
        mGraphUrl = graphurl;
    }

    QPixmap getGraphImage(){
#ifndef IS_WASM
        NetworkRequest net_request;
        net_request.setRequestMethod(NetworkRequest::Get);

        QByteArray image = net_request.loadSync(getGraphUrl());
        if(net_request.lastError() != ""){
            LogHelper::error("An error occurred (getGraphImage): " + net_request.lastError());
            return QPixmap();
        }
        QPixmap pixmap;
        pixmap.loadFromData(image,"PNG");
        return pixmap;
#else
        return QPixmap();
#endif
    }

    QString mModel;
    QString mGroup;
    QString mParamEQ;
    QString mGraphUrl;
};

Q_DECLARE_METATYPE(QueryResult)

#endif // AUTOEQCLIENT_H

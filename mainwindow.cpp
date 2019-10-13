#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ddccontext.h"
#include "addpoint.h"
#include "textpopup.h"
#include "calc.h"
#include "vdcimporter.h"
#include "undocmd.h"
#include <QFileDialog>
#include <QDebug>
#include <QAction>
#include <QMessageBox>
#include <vector>
#include <map>
#include "shiftfreq.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    undoStack = new QUndoStack(this);

    QAction* actionUndo = undoStack->createUndoAction(this, tr("Undo"));
    actionUndo->setShortcuts(QKeySequence::Undo);

    QAction* actionRedo = undoStack->createRedoAction(this, tr("Redo"));
    actionRedo->setShortcuts(QKeySequence::Redo);

    QAction* first = ui->menuEdit->actions().at(0);
    ui->menuEdit->insertAction(first,actionUndo);
    ui->menuEdit->insertAction(first,actionRedo);

    g_dcDDCContext = new class DDCContext;

    ui->listView_DDCPoints->setItemDelegate(new SaveItemDelegate());

    ui->listView_DDCPoints->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->graph->yAxis->setRange(QCPRange(-1, 1));
    ui->graph->xAxis->setRange(QCPRange(20, 24000));
    ui->graph->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->graph->xAxis->setLabel("Frequency (Hz)");

    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    ui->graph->xAxis->setTicker(logTicker);
    ui->graph->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->graph->rescaleAxes();
    connect(ui->graph, SIGNAL(mouseMove(QMouseEvent*)), this,SLOT(showPointToolTip(QMouseEvent*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//---Load/Save
void MainWindow::saveDDCProject()
{
    if(currentFile=="")
        saveAsDDCProject(true);
    else
        saveAsDDCProject(false,currentFile);
}
void MainWindow::saveAsDDCProject(bool ask,QString path,bool compatibilitymode)
{
    QString n("\n");
    QString fileName = path;
    QString dialogtitle = "Save VDC Project File";
    if(compatibilitymode) dialogtitle.append(" (Compatibility Mode)");
    if(ask)fileName = QFileDialog::getSaveFileName(this,dialogtitle, "", "ViPER DDC Project (*.vdcprj)");
    if (fileName != "" && fileName != nullptr)
    {
        QFileInfo fi(fileName);
        QString ext = fi.suffix();
        if(ext!="vdcprj")fileName.append(".vdcprj");
        setActiveFile(fileName);
        undoStack->clear();

        mtx.lock();
        std::vector<calibrationPoint_t> points;
        for (int i = 0; i < ui->listView_DDCPoints->rowCount(); i++)
        {
            calibrationPoint_t cal;
            cal.freq = (int)getValue(datatype::freq,i);
            cal.bw = getValue(datatype::bw,i);
            cal.gain = getValue(datatype::gain,i);
            cal.type = getType(i);
            points.push_back(cal);
        }
        mtx.unlock();
        writeProjectFile(points,fileName,compatibilitymode);
    }
}
void MainWindow::loadDDCProject()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open VDC Project File", "", "ViPER DDC Project (*.vdcprj)");
    if (fileName != "" && fileName != nullptr){
        mtx.lock();
        try
        {
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
                QMessageBox::warning(this,"Error","Cannot open file for reading");
                return;
            }
            QTextStream in(&file);
            QString str;
            clearPoint(false);
            setActiveFile(fileName);
            undoStack->clear();

            while (!in.atEnd())
                readLine_DDCProject(in.readLine().trimmed());
            file.close();
        }
        catch (const std::exception& e)
        {
            qWarning() << e.what();
            lock_actions=false;
            mtx.unlock();
            return;
        }
        mtx.unlock();
        ui->listView_DDCPoints->update();
        ui->listView_DDCPoints->sortItems(1,Qt::SortOrder::AscendingOrder);

        drawGraph();
    }
}
void MainWindow::readLine_DDCProject(QString str){
    lock_actions = true;
    if (str != nullptr || str != "")
    {
        if ((str.length() > 0) && !str.startsWith("#"))
        {
            QStringList strArray = str.split(',');
            if ((!strArray.empty()) && (strArray.length() == 3))
            {
                int result = 0;
                double num2 = 0.0;
                double num3 = 0.0;
                if ((sscanf(strArray[0].toUtf8().constData(), "%d", &result) == 1 &&
                     sscanf(strArray[1].toUtf8().constData(), "%lf", &num2) == 1) &&
                        sscanf(strArray[2].toUtf8().constData(), "%lf", &num3) == 1)
                {
                    if(result<=0||num2<0)return;
                    if(isnan(num2)||isnan(num3))return;
                    if(isinf(num2)||isinf(num3))return;

                    bool flag = false;
                    for (int i = 0; i < ui->listView_DDCPoints->rowCount(); i++)
                    {
                        if ((int)getValue(datatype::freq,i) == result)
                        {
                            flag = true;
                            break;
                        }
                    }
                    if (!flag)
                    {
                        insertData(biquad::Type::PEAKING,result,num2,num3);
                        g_dcDDCContext->AddFilter(biquad::Type::PEAKING,result, num3, num2, 44100.0,true);
                    }
                }
            }
            else if ((!strArray.empty()) && (strArray.length() == 4))
            {
                int result = 0;
                double num2 = 0.0;
                double num3 = 0.0;
                if ((sscanf(strArray[0].toUtf8().constData(), "%d", &result) == 1 &&
                     sscanf(strArray[1].toUtf8().constData(), "%lf", &num2) == 1) &&
                        sscanf(strArray[2].toUtf8().constData(), "%lf", &num3) == 1)
                {
                    if(result<=0||num2<0)return;
                    if(isnan(num2)||isnan(num3))return;
                    if(isinf(num2)||isinf(num3))return;

                    biquad::Type filtertype = stringToType(strArray[3].trimmed());

                    bool flag = false;
                    for (int i = 0; i < ui->listView_DDCPoints->rowCount(); i++)
                    {
                        if ((int)getValue(datatype::freq,i) == result)
                        {
                            flag = true;
                            break;
                        }
                    }
                    if (!flag)
                    {
                        insertData(filtertype,result,num2,num3);
                        g_dcDDCContext->AddFilter(filtertype,result, num3, num2, 44100.0,true);
                    }
                }
            }
        }
    }
    lock_actions = false;
}
//---Editor
void MainWindow::insertData(biquad::Type type,int freq,double band,double gain){
    ui->listView_DDCPoints->setSortingEnabled(false);
    QTableWidgetItem *c0 = new QTableWidgetItem();
    QTableWidgetItem *c1 = new QTableWidgetItem();
    QTableWidgetItem *c2 = new QTableWidgetItem();
    QTableWidgetItem *c3 = new QTableWidgetItem();
    c0->setData(Qt::DisplayRole, typeToString(type));
    c1->setData(Qt::DisplayRole, freq);
    c2->setData(Qt::DisplayRole, band);
    c3->setData(Qt::DisplayRole, gain);
    ui->listView_DDCPoints->insertRow ( ui->listView_DDCPoints->rowCount() );
    ui->listView_DDCPoints->setItem(ui->listView_DDCPoints->rowCount()-1, 0, c0);
    ui->listView_DDCPoints->setItem(ui->listView_DDCPoints->rowCount()-1, 1, c1);
    ui->listView_DDCPoints->setItem(ui->listView_DDCPoints->rowCount()-1, 2, c2);
    ui->listView_DDCPoints->setItem(ui->listView_DDCPoints->rowCount()-1, 3, c3);
    ui->listView_DDCPoints->setSortingEnabled(true);
}
void MainWindow::invertSelection(){
    if (ui->listView_DDCPoints->currentRow() >= 0 && ui->listView_DDCPoints->selectedItems().count() >= 1)
    {
        setActiveFile(currentFile,true);
        std::vector<calibrationPoint_t> cal_table;
        QItemSelectionModel *selected = ui->listView_DDCPoints->selectionModel();
        QModelIndexList list = selected->selectedRows();
        for (int i = 0; i < list.count(); i++)
        {
            QModelIndex index = list.at(i);
            int row = index.row();
            calibrationPoint_t cal;
            cal.freq = (int)getValue(datatype::freq,row);
            cal.bw = getValue(datatype::bw,row);
            cal.gain = getValue(datatype::gain,row);
            cal.type = getType(row);
            cal_table.push_back(cal);
        }
        QUndoCommand *invertCommand = new InvertCommand(ui->listView_DDCPoints,
                                                        g_dcDDCContext,cal_table,&mtx,&lock_actions,this);
        undoStack->push(invertCommand);
    }
    else
        QMessageBox::information(this,"Invert selection","No rows selected");
}
void MainWindow::shiftSelection(){
    if (ui->listView_DDCPoints->currentRow() >= 0 && ui->listView_DDCPoints->selectedItems().count() >= 1)
    {
        setActiveFile(currentFile,true);
        std::vector<calibrationPoint_t> cal_table;
        QItemSelectionModel *selected = ui->listView_DDCPoints->selectionModel();
        QModelIndexList list = selected->selectedRows();
        for (int i = 0; i < list.count(); i++)
        {
            QModelIndex index = list.at(i);
            int row = index.row();

            calibrationPoint_t cal;
            cal.freq = (int)getValue(datatype::freq,row);
            cal.bw = getValue(datatype::bw,row);
            cal.gain = getValue(datatype::gain,row);
            cal.type = getType(row);
            cal_table.push_back(cal);
        }

        shiftfreq* sf = new shiftfreq;
        sf->setRange(cal_table);
        if(sf->exec()){
            int shift = sf->getResult();
            QUndoCommand *shiftCommand = new ShiftCommand(ui->listView_DDCPoints,
                                                          g_dcDDCContext,shift,cal_table,&mtx,&lock_actions,this);
            undoStack->push(shiftCommand);
        }
    }
    else
        QMessageBox::information(this,"Shift selection","No rows selected");
}
void MainWindow::clearPoint(bool trackundo){
    if(trackundo){
        std::vector<calibrationPoint_t> cal_table;
        for (int i = 0; i < ui->listView_DDCPoints->rowCount(); i++)
        {
            calibrationPoint_t cal;
            cal.freq = (int)getValue(datatype::freq,i);
            cal.bw = getValue(datatype::bw,i);
            cal.gain = getValue(datatype::gain,i);
            cal.type = getType(i);
            cal_table.push_back(cal);
        }
        QUndoCommand *clearCommand = new ClearCommand(ui->listView_DDCPoints,
                                                      g_dcDDCContext,cal_table,&mtx,&lock_actions,this);
        undoStack->push(clearCommand);
    }
    else{
        lock_actions = true;

        g_dcDDCContext->ClearFilters();
        ui->listView_DDCPoints->clear();
        ui->listView_DDCPoints->setRowCount(0);
        ui->listView_DDCPoints->reset();
        ui->listView_DDCPoints->setHorizontalHeaderLabels(QStringList() << "Type" << "Frequency" << "Bandwidth" << "Gain");

        drawGraph();
        lock_actions = false;
    }
}
void MainWindow::editCell(QTableWidgetItem* item){
    if(lock_actions)return;
    setActiveFile(currentFile,true);
    int row = item->row();
    int result = 0;
    int nOldFreq = 0;
    double calibrationPointBandwidth = 0.0;
    double calibrationPointGain = 0.0;

    if(ui->listView_DDCPoints->rowCount() <= 0){
        qDebug() << "No data to edit!";
        return;
    }

    if ((sscanf(QString::number((int)getValue(datatype::freq,row)).toUtf8().constData(), "%d", &result) == 1 &&
         sscanf(QString::number(getValue(datatype::bw,row)).toUtf8().constData(), "%lf", &calibrationPointBandwidth) == 1) &&
            sscanf(QString::number(getValue(datatype::gain,row)).toUtf8().constData(), "%lf", &calibrationPointGain) == 1){

        ui->listView_DDCPoints->setSortingEnabled(false);
        nOldFreq = Global::old_freq;


        //Validate frequency value
        if(result < 0){
            ui->listView_DDCPoints->item(row,1)->setData(Qt::DisplayRole,0);
            QMessageBox::warning(this,"Warning","Frequency value '" + QString::number(result) + "' is too low (0.0 ~ 24000.0)");
            result = 0;
        }
        else if(result > 24000){
            ui->listView_DDCPoints->item(row,1)->setData(Qt::DisplayRole,24000);
            QMessageBox::warning(this,"Warning","Frequency value '" + QString::number(result) + "' is too high (0.0 ~ 24000.0)");
            result = 24000;
        }

        //Validate bandwidth value
        else if(calibrationPointBandwidth < 0){
            ui->listView_DDCPoints->item(row,2)->setData(Qt::DisplayRole,(double)0);
            QMessageBox::warning(this,"Warning","Bandwidth value '" + QString::number(calibrationPointBandwidth) + "' is too low (0.0 ~ 100.0)");
            calibrationPointBandwidth = 0;
        }
        else if(calibrationPointBandwidth > 100){
            ui->listView_DDCPoints->item(row,2)->setData(Qt::DisplayRole,(double)100);
            QMessageBox::warning(this,"Warning","Bandwidth value '" + QString::number(calibrationPointBandwidth) + "' is too high (0.0 ~ 100.0)");
            calibrationPointBandwidth = 100;
        }

        //Validate gain value
        else if(calibrationPointGain < -40){
            ui->listView_DDCPoints->item(row,3)->setData(Qt::DisplayRole,(double)-40);
            QMessageBox::warning(this,"Warning","Gain value '" + QString::number(calibrationPointGain) + "' is too low (-40.0 ~ 40.0)");
            calibrationPointGain = -40;
        }
        else if(calibrationPointGain > 40){
            ui->listView_DDCPoints->item(row,3)->setData(Qt::DisplayRole,(double)40);
            QMessageBox::warning(this,"Warning","Gain value '" + QString::number(calibrationPointGain) + "' is too high (-40.0 ~ 40.0)");
            calibrationPointGain = 40;
        }

        //Check for duplicate frequencies
        for (int i = 0; i < ui->listView_DDCPoints->rowCount(); i++)
        {
            if ((int)getValue(datatype::freq,i) == (int)getValue(datatype::freq,row) && row != i)
            {
                ui->listView_DDCPoints->item(row,1)->setData(Qt::DisplayRole,Global::old_freq);
                QMessageBox::warning(this,"Error","Point '" + QString::number((int)getValue(datatype::freq,i)) + "' already exists");
                return;
            }
        }

        calibrationPoint_t cal;
        cal.freq = result;
        cal.bw = calibrationPointBandwidth;
        cal.gain = calibrationPointGain;
        cal.type = getType(row);
        calibrationPoint_t oldcal;
        oldcal.freq = nOldFreq;
        oldcal.bw = Global::old_bw;
        oldcal.gain = Global::old_gain;
        oldcal.type = Global::old_type;

        QUndoCommand *editCommand = new EditCommand(ui->listView_DDCPoints,g_dcDDCContext,
                                                    row,cal,oldcal,&mtx,&lock_actions,this);
        undoStack->push(editCommand);

        drawGraph();
    }
    else qDebug() << "Invalid input data";
}
void MainWindow::addPoint(){
    addpoint *dlg = new addpoint;
    if(dlg->exec()){
        setActiveFile(currentFile,true);
        addp_response_t rawdata = dlg->returndata();

        calibrationPoint_t cal;
        cal.freq = (int)rawdata.values.at(0);
        cal.bw = rawdata.values.at(1);
        cal.gain = rawdata.values.at(2);
        cal.type = stringToType(rawdata.filtertype);

        for (int i = 0; i < ui->listView_DDCPoints->rowCount(); i++)
        {
            if ((int)getValue(datatype::freq,i) == cal.freq)
            {
                QMessageBox::warning(this,"Error","Point already exists");
                return;
            }
        }

        QUndoCommand *addCommand = new AddCommand(ui->listView_DDCPoints,
                                                  g_dcDDCContext,cal,&mtx,&lock_actions,this);
        undoStack->push(addCommand);
        drawGraph();
    }
}
void MainWindow::removePoint(){
    lock_actions=true;
    if (ui->listView_DDCPoints->currentRow() >= 0)
    {
        setActiveFile(currentFile,true);
        ui->listView_DDCPoints->setSortingEnabled(false);
        std::vector<int> removeRows;
        std::vector<calibrationPoint_t> cal_table;
        QItemSelectionModel *selected = ui->listView_DDCPoints->selectionModel();
        QModelIndexList list = selected->selectedRows();
        for (int i = 0; i < list.count(); i++)
        {
            QModelIndex index = list.at(i);
            int row = index.row();
            removeRows.push_back(row);

            calibrationPoint_t cal;
            cal.freq = (int)getValue(datatype::freq,i);
            cal.bw = getValue(datatype::bw,i);
            cal.gain = getValue(datatype::gain,i);
            cal.type = getType(i);
            cal_table.push_back(cal);
        }
        QUndoCommand *removeCommand = new RemoveCommand(ui->listView_DDCPoints,
                                                        g_dcDDCContext,removeRows,list,cal_table,&mtx,&lock_actions,this);
        undoStack->push(removeCommand);

        ui->listView_DDCPoints->setSortingEnabled(true);
        ui->listView_DDCPoints->sortItems(1);
        drawGraph();
    }
    lock_actions=false;
}
void MainWindow::drawGraph(){
    int bandCount = 240;
    std::vector<float> responseTable = g_dcDDCContext->GetResponseTable(bandCount, 44100.0);
    if (responseTable.size()<=0)
    {
        return;
    }
    float num2 = 0.0f;
    for (size_t i = 0; i < (size_t)bandCount; i++)
        if (abs(responseTable.at(i)) > num2)
            num2 = abs(responseTable.at(i));
    if (num2 <= 1E-08f)
    {
        qDebug() << "Cannot draw graph. Currently at line: " << __LINE__;
        ui->graph->clearItems();
        ui->graph->clearGraphs();
    }
    else
    {
        if (num2 > 24.0f)
        {
            float num4 = 24.0f / num2;
            for (size_t n = 0; n < (size_t)bandCount; n++)
                responseTable.at(n) *= num4;
        }
        for (size_t k = 0; k < (size_t)bandCount; k++)
            responseTable.at(k) /= 24.0f;

        ui->graph->clearPlottables();
        ui->graph->clearItems();
        ui->graph->clearGraphs();
        ui->graph->yAxis->setRange(QCPRange(-1, 1));
        QCPGraph *plot = ui->graph->addGraph();

        for (size_t m = 0; m < (size_t)bandCount; m++)
        {
            plot->addData(m*100,(double)responseTable.at(m)); //m * 100 -> to fit the scale to 24000hz
            ui->graph->xAxis->setRange(QCPRange(20, m*100));
        }
    }
    ui->graph->replot();
}
void MainWindow::toggleGraph(bool state){
    ui->graphBox->setVisible(!state);
}
void MainWindow::ScreenshotGraph(){
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save Screenshot", "", "PNG screenshot (*.png)");
    if (fileName != "" && fileName != nullptr){
        QFileInfo fi(fileName);
        QString ext = fi.suffix();
        if(ext!="png")fileName.append(".png");
        ui->graph->saveJpg(fileName);
    }
}
//---Dialogs
void MainWindow::showIntroduction(){
    QString data = "Unable to open HTML file";
    QFile file(":/html/introduction.html");
    if(!file.open(QIODevice::ReadOnly))
        qDebug()<<"Unable to open HTML file. Line: " << __LINE__;
    else
        data = file.readAll();
    file.close();
    TextPopup *t = new TextPopup(data);
    t->show();
}
void MainWindow::showHelp(){
    QString data = "Unable to open HTML file";
    QFile file(":/html/help.html");
    if(!file.open(QIODevice::ReadOnly))
        qDebug()<<"Unable to open HTML file. Line: " << __LINE__;
    else
        data = file.readAll();
    file.close();
    TextPopup *t = new TextPopup(data);
    t->show();
}
void MainWindow::showKeycombos(){
    QString data = "Unable to open HTML file";
    QFile file(":/html/keycombos.html");
    if(!file.open(QIODevice::ReadOnly))
        qDebug()<<"Unable to open HTML file. Line: " << __LINE__;
    else
        data = file.readAll();
    file.close();
    TextPopup *t = new TextPopup(data);
    t->show();
}
void MainWindow::showCalc(){
    calc *t = new calc();
    t->show();
    QMessageBox::information(this,"",QString::number(getValue(datatype::freq,0))+","+
                             QString::number(getValue(datatype::freq,1))+","+
                             QString::number(getValue(datatype::freq,2)));
}
void MainWindow::showUndoView(){
    undoView = new QUndoView(undoStack);
    undoView->setWindowTitle(tr("Undo History"));
    undoView->show();
    undoView->setAttribute(Qt::WA_QuitOnClose, false);
}

//---Import/Export
void MainWindow::exportCompatVDCProj(){
    saveAsDDCProject(true,"",true);
}
void MainWindow::importVDC(){
    int i;
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open VDC", "", "VDC file (*.vdc)");
    if (fileName != "" && fileName != nullptr){
        clearPoint(false);
        setActiveFile("");
        undoStack->clear();

        mtx.lock();

        QString str;
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox::warning(this,"Error","Cannot open file for reading");
            return;
        }
        QTextStream in(&file);
        QByteArray ba = in.readAll().toLatin1();
        char* textString = ba.data();

        DirectForm2 **df441, **df48;
        int sosCount = DDCParser(textString, &df441, &df48);
        char *vdcprj = VDC2vdcprj(df48, 48000.0, sosCount);

        QString line;
        QString data(vdcprj);
        QTextStream stream(&data);
        while (stream.readLineInto(&line)) {
            readLine_DDCProject(line);
        }
        stream.seek(0);

        free(vdcprj);
        for (i = 0; i < sosCount; i++)
        {
            free(df441[i]);
            free(df48[i]);
        }
        free(df441);
        free(df48);
        mtx.unlock();
        drawGraph();
    }
}
void MainWindow::exportVDC()
{
    QString n("\n");
    std::list<double> p1 = g_dcDDCContext->ExportCoeffs(44100.0);
    std::list<double> p2 = g_dcDDCContext->ExportCoeffs(48000.0);

    if (p1.empty() || p2.empty())
    {
        QMessageBox::warning(this,"Error","Failed to export to VDC");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save VDC", "", "VDC File (*.vdc)");
    if (fileName != "" && fileName != nullptr)
    {
        QFileInfo fi(fileName);
        QString ext = fi.suffix();
        if(ext!="vdc")fileName.append(".vdc");
        mtx.lock();
        try
        {
            QFile caFile(fileName);
            caFile.open(QIODevice::WriteOnly | QIODevice::Text);

            if(!caFile.isOpen()){
                qDebug() << "- Error, unable to open" << "outputFilename" << "for output";
            }
            QTextStream outStream(&caFile);
            outStream << "SR_44100:";

            std::vector<double> v1;
            for (double const &d: p1)
                v1.push_back(d);
            std::vector<double> v2;
            for (double const &d: p2)
                v2.push_back(d);

            for (size_t i = 0; i < v1.size(); i++)
            {
                outStream << qSetRealNumberPrecision(16) << v1.at(i);
                if (i != (v1.size() - 1))
                    outStream << ",";
            }
            outStream << n;
            outStream << "SR_48000:";

            for (size_t i = 0; i < v2.size(); i++)
            {
                outStream << qSetRealNumberPrecision(16) << v2.at(i);
                if (i != (v2.size() - 1))
                    outStream << ",";
            }

            outStream << n;
            caFile.close();
        }
        catch (const std::exception& e)
        {
            qWarning() << e.what();
            mtx.unlock();
            return;
        }
        mtx.unlock();
    }
}
void MainWindow::importParametricAutoEQ(){
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Import AutoEQ config 'ParametricEQ.txt'", "", "AutoEQ ParametricEQ.txt (*ParametricEQ.txt);;All files (*.*)");
    if (fileName != "" && fileName != nullptr){
        ui->listView_DDCPoints->clear();
        clearPoint(false);
        setActiveFile("");
        undoStack->clear();

        std::vector<calibrationPoint_t> points = parseParametricEQ(fileName);
        if(points.size() < 1){
            QMessageBox::warning(this,"Error","Unable to convert this file; no data found: " + fileName);
            return;
        }

        for(size_t i=0;i<points.size();i++){
            calibrationPoint_t cal = points.at(i);
            bool flag = false;
            for (int i = 0; i < ui->listView_DDCPoints->rowCount(); i++)
            {
                if ((int)getValue(datatype::freq,i) == cal.freq)
                {
                    flag = true;
                    break;
                }
            }
            if (!flag)
            {
                lock_actions = true;
                insertData(biquad::Type::PEAKING,cal.freq,(double)cal.bw,(double)cal.gain);
                lock_actions = false;
                g_dcDDCContext->AddFilter(biquad::Type::PEAKING,cal.freq, (double)cal.gain, (double)cal.bw, 44100.0,true);
            }
        }
        ui->listView_DDCPoints->sortItems(1,Qt::SortOrder::AscendingOrder);
        drawGraph();
    }
}
//---Parser/Writer
bool MainWindow::writeProjectFile(std::vector<calibrationPoint_t> points,QString fileName,bool compatibilitymode){
    QString n("\n");
    QFile caFile(fileName);
    caFile.open(QIODevice::WriteOnly | QIODevice::Text);

    if(!caFile.isOpen()){
        qDebug() << "Error, unable to open" << "outputFilename" << "for output";
        return false;
    }
    QTextStream outStream(&caFile);
    if(compatibilitymode){
        outStream << "# DDCToolbox Project File, v1.0.0.0 (@ThePBone)" + n;
        outStream << n;
        for (size_t i = 0; i < points.size(); i++)
        {
            calibrationPoint_t cal = points.at(i);
            outStream << "# Calibration Point " + QString::number(i + 1) + n;
            outStream << QString::number(cal.freq) + "," + QString::number(cal.bw) + "," + QString::number(cal.gain) + n;
        }
    }else{
        outStream << "# DDCToolbox Project File, v2.0.0.0 (@ThePBone)" + n;
        outStream << n;
        for (size_t i = 0; i < points.size(); i++)
        {
            calibrationPoint_t cal = points.at(i);
            outStream << "# Calibration Point " + QString::number(i + 1) + n;
            outStream << QString::number(cal.freq) + "," + QString::number(cal.bw) + "," + QString::number(cal.gain) + "," + typeToString(cal.type) + n;
        }
    }
    outStream << n;
    outStream << "#File End" + n;
    caFile.close();
    return true;
}
std::vector<calibrationPoint_t> MainWindow::parseParametricEQ(QString path){
    std::vector<calibrationPoint_t> points;
    QString str;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return points;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        str = in.readLine().trimmed();
        if (str != nullptr || str != "")
        {
            if ((str.length() > 0) && !str.startsWith("#"))
            {
                QString strPart2 = str.split(':')[1].trimmed();
                QStringList lineParts = strPart2.split(" ");
                /**
                  [0] "ON"
                  [1] "PK"
                  [2] "Fc"
                  [3] <Freq,INT>
                  [4] "Hz"
                  [5] "Gain"
                  [6] <Gain,FLOAT>
                  [7] "dB"
                  [8] "Q"
                  [9] <Q-Value,FLOAT>
                                      **/
                if ((!lineParts.empty()) && (lineParts.length() == 10))
                {
                    int freq = lineParts[3].toInt();
                    float gain = lineParts[6].toFloat();
                    float q = lineParts[9].toFloat();
                    if(freq < 0)continue;

                    //Convert Q to BW
                    float QQ1st = ((2*q*q)+1)/(2*q*q);
                    float QQ2nd = pow(2*QQ1st,2)/4;
                    float bw = round(1000000*log(QQ1st+sqrt(QQ2nd-1))/log(2))/1000000;

                    calibrationPoint_t cal;
                    cal.freq = freq;
                    cal.bw = bw;
                    cal.gain = gain;
                    cal.type = biquad::Type::PEAKING; //TODO: add filtertype to eapo/autoeq parser
                    points.push_back(cal);
                }
            }
        }
    }
    file.close();
}
//---Batch conversion
void MainWindow::batch_vdc2vdcprj(){
    QStringList filenames = QFileDialog::getOpenFileNames(this,"Select all VDC files to convert",QDir::currentPath(),tr("VDC files (*.vdc)") );
    if( !filenames.isEmpty() )
    {
        QMessageBox::information(this,"Note",QString::number((int)filenames.count()) +
                                 " files will be converted.\nYou will now be prompted to select an output directory.");
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Output-Directory"),
                                                        "",
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

        if(dir=="")return;
        for (int l=0;l<(int)filenames.count();l++){
            int i = 0;

            QString str;
            QFile file(filenames.at(l));
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
                QMessageBox::warning(this,"Error","Cannot open file " + filenames.at(l) + " for reading");
                continue;
            }
            QTextStream in(&file);
            QByteArray ba = in.readAll().toLatin1();
            char* textString = ba.data();

            DirectForm2 **df441, **df48;
            int sosCount = DDCParser(textString, &df441, &df48);
            char *vdcprj = VDC2vdcprj(df48, 48000.0, sosCount);


            QFileInfo fi(filenames.at(l));
            QString out = fi.completeBaseName();
            QFile qFile(QDir(dir).filePath(fi.completeBaseName()+".vdcprj"));
            if (qFile.open(QIODevice::WriteOnly)) {
                QTextStream out(&qFile); out << QString(vdcprj);
                qFile.close();
            }

            free(vdcprj);
            for (i = 0; i < sosCount; i++)
            {
                free(df441[i]);
                free(df48[i]);
            }
            free(df441);
            free(df48);
        }
        QMessageBox::information(this,"Note","Conversion finished!\nYou can find the files here:\n"+dir);
    }
}
void MainWindow::batch_parametric2vdcprj(){
    QStringList filenames = QFileDialog::getOpenFileNames(this,"Select all AutoEQ ParametricEQ.txt files to convert",QDir::currentPath(),tr("AutoEQ ParametricEQ.txt (*ParametricEQ.txt);;All files (*.*)") );
    if( !filenames.isEmpty() )
    {
        QMessageBox::information(this,"Note",QString::number((int)filenames.count()) +
                                 " files will be converted.\nYou will now be prompted to select an output directory.");
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Output-Directory"),
                                                        "",
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        if(dir=="")return;
        for (int l=0;l<(int)filenames.count();l++){
            int i = 0;
            std::vector<calibrationPoint_t> points = parseParametricEQ(filenames.at(l));
            if(points.size() < 1){
                QMessageBox::warning(this,"Error","Unable to convert this file: " + filenames.at(l));
                return;
            }
            QFileInfo fi(filenames.at(l));
            if(!writeProjectFile(points,QDir(dir).filePath(fi.completeBaseName()+".vdcprj"),false))
                QMessageBox::warning(this,"Error","Cannot write file at: " + QDir(dir).filePath(fi.completeBaseName()+".vdcprj"));
        }
        QMessageBox::information(this,"Note","Conversion finished!\nYou can find the files here:\n"+dir);
    }
}
//---Misc
///Tooltip with x-value while hovering graph
void MainWindow::showPointToolTip(QMouseEvent *event)
{
    int x = (int)ui->graph->xAxis->pixelToCoord(event->pos().x());
    if(x<0||x>24000)return;
    //int y = this->yAxis->pixelToCoord(event->pos().y());
    ui->graph->setToolTip(QString("%1Hz").arg(x));

}
//---Session
void MainWindow::setActiveFile(QString path,bool unsaved){
    QFileInfo fi(path);
    currentFile = path;
    if(path=="")
        setWindowTitle("DDC Toolbox");
    else{
        if(unsaved) setWindowTitle("DDC Toolbox - " + fi.fileName() + "*");
        else setWindowTitle("DDC Toolbox - " + fi.fileName());
    }
}
void MainWindow::closeProject(){
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "DDC Toolbox", "Are you sure? All unsaved changes will be lost.",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        setActiveFile("");
        clearPoint(false);
        undoStack->clear();
    }
}

double MainWindow::getValue(datatype dat,int row){
    switch(dat){
    case type:
        return ui->listView_DDCPoints->item(row,0)->data(Qt::DisplayRole).toInt();
    case freq:
        return ui->listView_DDCPoints->item(row,1)->data(Qt::DisplayRole).toInt();
    case bw:
        return ui->listView_DDCPoints->item(row,2)->data(Qt::DisplayRole).toDouble();
    case gain:
        return ui->listView_DDCPoints->item(row,3)->data(Qt::DisplayRole).toDouble();
    }
}
biquad::Type MainWindow::getType(int row){
    QString type = ui->listView_DDCPoints->item(row,0)->data(Qt::DisplayRole).toString();
    return stringToType(type);
}

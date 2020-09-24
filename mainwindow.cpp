#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ddccontext.h"
#include "dialog/addpoint.h"
#include "dialog/textpopup.h"
#include "dialog/calc.h"
#include "utils/undocmd.h"
#include <QFileDialog>
#include <QDebug>
#include <QAction>
#include <QMessageBox>
#include <QtGlobal>
#include <vector>
#include <map>
#include <QMessageBox>
#include "dialog/shiftfreq.h"
#include "utils/delegate.h"
#include "item/customfilteritem.h"
#include "item/customfilterfactory.h"
#include "io/projectmanager.h"
#include <dialog/autoeqselector.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    undoStack = new QUndoStack(this);


    QMainWindow* subMainWindow = new QMainWindow(0);
    QHBoxLayout* lay = new QHBoxLayout;
    subMainWindow->setWindowTitle("sub-mainwindow") ;
    subMainWindow->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea,
                                 ui->magnitude_dock);
    subMainWindow->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea,
                                 ui->phase_dock);
    subMainWindow->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea,
                                 ui->groupdelay_dock);
    subMainWindow->tabifyDockWidget(ui->magnitude_dock,ui->phase_dock);
    subMainWindow->tabifyDockWidget(ui->phase_dock,ui->groupdelay_dock);
    subMainWindow->setDockOptions(DockOption::VerticalTabs
                                  | DockOption::AllowTabbedDocks
                                  | DockOption::AnimatedDocks);
    ui->magnitude_dock->show();
    ui->magnitude_dock->raise();
    lay->addWidget(subMainWindow);
    ui->plotcontainer->setLayout(lay);

#ifndef IS_WASM
    m_updater = QSimpleUpdater::getInstance();
#else
    ui->actionCheck_for_updates->setVisible(false);
    ui->actionDownload_from_AutoEQ->setVisible(false);
    ui->menuBatch_Conversion_2->menuAction()->setVisible(false);
#endif

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

    connect(ui->listView_DDCPoints->selectionModel(),
            static_cast<void (QItemSelectionModel::*)(const QItemSelection &, const QItemSelection &)>(&QItemSelectionModel::selectionChanged),
            this,[=](){
        if(lock_actions)return;
        drawGraph(GraphType::all,true);
    });
    connect(ui->actionReset_plot_layout,&QAction::triggered,this,[subMainWindow,this]{
        ui->magnitude_dock->setFloating(false);
        ui->phase_dock->setFloating(false);
        ui->groupdelay_dock->setFloating(false);
        subMainWindow->tabifyDockWidget(ui->magnitude_dock,ui->phase_dock);
        subMainWindow->tabifyDockWidget(ui->phase_dock,ui->groupdelay_dock);
        subMainWindow->setDockOptions(DockOption::VerticalTabs
                                      | DockOption::AllowTabbedDocks
                                      | DockOption::AnimatedDocks);
        ui->magnitude_dock->show();
        ui->magnitude_dock->raise();
    });

    ui->graph->setMode(FrequencyPlot::PlotType::magnitude,this);
    ui->gdelay_graph->setMode(FrequencyPlot::PlotType::group_delay,this);
    ui->phase_graph->setMode(FrequencyPlot::PlotType::phase_response,this);
    createLanguageMenu();

    QApplication::setPalette(this->style()->standardPalette());
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
    auto _saveProject = [this,compatibilitymode](QString path, bool updateActiveFile){
        if (path.isEmpty())
            return;

        QFileInfo fi(path);
        QString ext = fi.suffix();
        if(ext!="vdcprj") path.append(".vdcprj");

        if(updateActiveFile)
            setActiveFile(path);

        undoStack->clear();

        mtx.lock();
        std::vector<calibrationPoint_t> points;
        for (int i = 0; i < ui->listView_DDCPoints->rowCount(); i++)
        {
            calibrationPoint_t cal;
            cal.freq = (int)getValue(DataType::freq,i);
            cal.bw = getValue(DataType::bw,i);
            cal.gain = getValue(DataType::gain,i);
            cal.type = getType(i);
            cal.id = getId(i);
            if(cal.type == biquad::CUSTOM){
                cal.custom441 = ((CustomFilterItem*)ui->listView_DDCPoints->cellWidget(i,3))->getCoefficients(false);
                cal.custom48 = ((CustomFilterItem*)ui->listView_DDCPoints->cellWidget(i,3))->getCoefficients(true);
            }
            points.push_back(cal);
        }
        mtx.unlock();
        ProjectManager::writeProjectFile(points, path, compatibilitymode);
    };

#ifdef IS_WASM
    Q_UNUSED(ask);
    _saveProject(".tmp_proj.vdcprj", false);
    QFile file(".tmp_proj.vdcprj");
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QByteArray blob = file.readAll();
    file.close();

    QFileDialog::saveFileContent(blob, currentFile.isEmpty() ? "project.vdcprj" : QFileInfo(currentFile).fileName());
#else
    QString dialogtitle = tr("Save VDC Project File");
    if(compatibilitymode) dialogtitle.append(" " + tr("(Compatibility Mode)"));
    if(ask){
        path = QFileDialog::getSaveFileName(this,dialogtitle, "", "ViPER DDC Project (*.vdcprj)");
        _saveProject(path, true);
    }
    else _saveProject(path, true);
#endif
}

void MainWindow::loadDDCProject()
{
    auto _loadProject = [this](const QString& file){
        if(file.length() < 1) return;
        auto project_data = ProjectManager::readProjectFile(file);
        if(project_data.size() > 0){
            mtx.lock();

            clearPoint(false);
            setActiveFile(file);
            undoStack->clear();

            lock_actions = true;
            ui->listView_DDCPoints->setSortingEnabled(false);

            for(size_t i = 0; i < project_data.size(); i++){
                calibrationPoint_t cal = project_data.at(i);
                uint32_t id = insertData(cal.type,cal.freq,cal.bw,cal.gain,false);
                if(cal.type == biquad::CUSTOM){
                    newCustomFilter(cal.custom441,cal.custom48,ui->listView_DDCPoints,ui->listView_DDCPoints->rowCount()-1);
                    g_dcDDCContext->AddFilter(id,cal.custom441,cal.custom48);
                } else {
                    g_dcDDCContext->AddFilter(id,cal.type,cal.freq, cal.gain, cal.bw, 48000.0,true);
                }
            }

            ui->listView_DDCPoints->setSortingEnabled(true);
            ui->listView_DDCPoints->sortItems(1);
            ui->listView_DDCPoints->update();
            ui->listView_DDCPoints->sortItems(1,Qt::SortOrder::AscendingOrder);
            lock_actions = false;
            mtx.unlock();

            drawGraph();
            emit loadFinished();
        }
        else
            QMessageBox::critical(this,"Error","Invalid project file or inaccessible file");

    };

#ifdef IS_WASM
    auto fileContentReady = [this,_loadProject](const QString &fileName, const QByteArray &fileContent) {
ui->groupBox->setTitle(fileContent);
        QString name = (fileName.isEmpty() ? "project.vdcprj" : fileName);
        QFile file(name);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            stream << fileContent;
            file.close();

            _loadProject(name);
        }
    };
    QFileDialog::getOpenFileContent("ViPER DDC Project (*.vdcprj)",  fileContentReady);
#else
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open VDC Project File"), "", tr("ViPER DDC Project (*.vdcprj)"));
    _loadProject(fileName);
#endif
}
void MainWindow::closeProject(){
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "DDC Toolbox", tr("Are you sure? All unsaved changes will be lost."),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        setActiveFile("");
        clearPoint(false);
        drawGraph();
        undoStack->clear();
    }
}
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

//---Editor-Actions
void MainWindow::addPoint(){
    addpoint *dlg = new addpoint;
    if(dlg->exec()){
        setActiveFile(currentFile,true);
        calibrationPoint_t cal = dlg->returndata();
        cal.id = g_dcDDCContext->GenerateId();
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
        QItemSelectionModel *selected = ui->listView_DDCPoints->selectionModel();
        QModelIndexList list = selected->selectedRows();
        for (int i = 0; i < list.count(); i++)
        {
            QModelIndex index = list.at(i);
            int row = index.row();
            removeRows.push_back(row);
        }
        QUndoCommand *removeCommand = new RemoveCommand(ui->listView_DDCPoints,
                                                        g_dcDDCContext,removeRows,list,&mtx,&lock_actions,this);
        undoStack->push(removeCommand);

        ui->listView_DDCPoints->setSortingEnabled(true);
        ui->listView_DDCPoints->sortItems(1);
        drawGraph();
    }
    lock_actions=false;
}
void MainWindow::clearPoint(bool trackundo){
    if(trackundo){
        std::vector<calibrationPoint_t> cal_table;
        for (int i = 0; i < ui->listView_DDCPoints->rowCount(); i++)
        {
            calibrationPoint_t cal;
            cal.freq = (int)getValue(DataType::freq,i);
            cal.bw = getValue(DataType::bw,i);
            cal.gain = getValue(DataType::gain,i);
            cal.type = getType(i);
            cal.id = getId(i);
            if(cal.type == biquad::CUSTOM){
                cal.custom441 = ((CustomFilterItem*)ui->listView_DDCPoints->cellWidget(i,3))->getCoefficients(false);
                cal.custom48 = ((CustomFilterItem*)ui->listView_DDCPoints->cellWidget(i,3))->getCoefficients(true);
            }
            cal_table.push_back(cal);
        }
        QUndoCommand *clearCommand = new ClearCommand(ui->listView_DDCPoints,
                                                      g_dcDDCContext,cal_table,&mtx,&lock_actions,this);
        undoStack->push(clearCommand);
    }
    else{
        lock_actions = true;

        g_dcDDCContext->ClearFilters();
        (new tableproxy(ui->listView_DDCPoints))->clearAll();
        Global::old_freq = 0;
        Global::old_bw = 0;
        Global::old_gain = 0;
        Global::old_type = (biquad::Type)0;
        Global::old_custom441 = defaultCustomFilter();
        Global::old_custom48 = defaultCustomFilter();

        drawGraph();
        lock_actions = false;
    }
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
            cal.freq = (int)getValue(DataType::freq,row);
            cal.bw = getValue(DataType::bw,row);
            cal.gain = getValue(DataType::gain,row);
            cal.type = getType(row);
            cal.id = getId(row);
            if(cal.type != biquad::CUSTOM)
                cal_table.push_back(cal);
        }
        QUndoCommand *invertCommand = new InvertCommand(ui->listView_DDCPoints,
                                                        g_dcDDCContext,cal_table,&mtx,&lock_actions,this);
        undoStack->push(invertCommand);
    }
    else
        QMessageBox::information(this,tr("Invert selection"),tr("No rows selected"));
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
            cal.freq = (int)getValue(DataType::freq,row);
            cal.bw = getValue(DataType::bw,row);
            cal.gain = getValue(DataType::gain,row);
            cal.type = getType(row);
            cal.id = getId(row);
            if(cal.type != biquad::CUSTOM && cal.type != biquad::UNITY_GAIN)
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
        QMessageBox::information(this,tr("Shift selection"),tr("No rows selected"));
}
void MainWindow::CheckStability(){
    if(lock_actions)return;
    int unstableCount = 0;
    QString stringbuilder("");
    for (int i = 0; i < ui->listView_DDCPoints->rowCount(); i++){
        int freq = (int)getValue(DataType::freq,i);
        QString filtertype = typeToString(getType(i));
        const biquad* filter = g_dcDDCContext->GetFilter(freq);
        if(filter != nullptr){
            int stability = filter->IsStable();
            if(stability == 0){
                unstableCount++;
                stringbuilder += QString(tr("Fatal error: Pole of %1 at %2Hz (row %3) outside the unit circle\n")).arg(filtertype).arg(freq).arg(i+1);
            }
            else if(stability == 2){
                unstableCount++;
                stringbuilder += QString(tr("Warning: Pole of %1 at %2Hz (row %3) approach to unit circle\n")).arg(filtertype).arg(freq).arg(i+1);
            }
        }
    }
    if(unstableCount <= 0)
        QMessageBox::information(this,tr("Stability check"),QString(tr("All filters appear to be stable.")));
    else{
        QMessageBox::warning(this,tr("Stability check"),QString(tr("One or more filters are potentially unstable:\n\n%1\nPlease review these filter and run this check again.")).arg(stringbuilder));
    }
}

//---Editor-Internal
void MainWindow::editCell(QTableWidgetItem* item){
    if(lock_actions)return;
    setActiveFile(currentFile,true);
    int row = item->row();
    int result = 0;
    int nOldFreq = 0;
    double calibrationPointBandwidth = 0.0;
    double calibrationPointGain = 0.0;

    if(ui->listView_DDCPoints->rowCount() <= 0){
        return;
    }
    if(getType(row)==biquad::CUSTOM){
        if(sender()==ui->listView_DDCPoints){
            Global::old_freq = (int)getValue(DataType::freq,row);
            //dirty check if custom element is new created by addpoint.
            //If true, make sure we set the initial old value to the current frequency value for compatibility
        }
        //ui->listView_DDCPoints->setSpan(row,2,1,2);

        calibrationPoint_t cal;
        cal.id = getId(row);
        cal.freq = (int)getValue(DataType::freq,row);
        cal.type = getType(row);
        cal.bw = getValue(DataType::bw,row);
        cal.gain = getValue(DataType::bw,row);
        if(ui->listView_DDCPoints->cellWidget(row,3)==nullptr ){
            cal.custom441 = defaultCustomFilter();
            cal.custom48 = defaultCustomFilter();
        }
        else{
            cal.custom441 = ((CustomFilterItem*)ui->listView_DDCPoints->cellWidget(row,3))->getCoefficients(false);
            cal.custom48 = ((CustomFilterItem*)ui->listView_DDCPoints->cellWidget(row,3))->getCoefficients(true);
        }

        calibrationPoint_t oldcal;
        oldcal.id = getId(row);
        oldcal.freq = Global::old_freq;
        oldcal.bw = Global::old_bw;
        oldcal.gain = Global::old_gain;
        oldcal.custom441 = Global::old_custom441;
        oldcal.custom48 = Global::old_custom48;
        oldcal.type = Global::old_type;

        QUndoCommand *editCommand = new EditCommand(ui->listView_DDCPoints,g_dcDDCContext,
                                                    row,cal,oldcal,&mtx,&lock_actions,this);
        undoStack->push(editCommand);
        drawGraph();
    }
    else{
        //ui->listView_DDCPoints->setSpan(row,2,1,1);
        ui->listView_DDCPoints->removeCellWidget(row,3);
        if ((sscanf(QString::number((int)getValue(DataType::freq,row)).toUtf8().constData(), "%d", &result) == 1 &&
             sscanf(QString::number(getValue(DataType::bw,row)).toUtf8().constData(), "%lf", &calibrationPointBandwidth) == 1) &&
                sscanf(QString::number(getValue(DataType::gain,row)).toUtf8().constData(), "%lf", &calibrationPointGain) == 1){

            ui->listView_DDCPoints->setSortingEnabled(false);

            //Validate frequency value
            if(result < 0){
                ui->listView_DDCPoints->item(row,1)->setData(Qt::DisplayRole,0);
                QMessageBox::warning(this,tr("Warning"),tr("Frequency value '%1' is too low (0.0 ~ 24000.0)").arg(result));
                result = 0;
            }
            else if(result > 24000){
                ui->listView_DDCPoints->item(row,1)->setData(Qt::DisplayRole,24000);
                QMessageBox::warning(this,tr("Warning"),tr("Frequency value '%1' is too high (0.0 ~ 24000.0)"));
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
                QMessageBox::warning(this,tr("Warning"),tr("Gain value '%1' is too low (-40.0 ~ 40.0)").arg(calibrationPointGain));
                calibrationPointGain = -40;
            }
            else if(calibrationPointGain > 40){
                ui->listView_DDCPoints->item(row,3)->setData(Qt::DisplayRole,(double)40);
                QMessageBox::warning(this,tr("Warning"),tr("Gain value '%1' is too high (-40.0 ~ 40.0)").arg(calibrationPointGain));
                calibrationPointGain = 40;
            }

            calibrationPoint_t cal;
            cal.id = getId(row);
            cal.freq = result;
            cal.bw = calibrationPointBandwidth;
            cal.gain = calibrationPointGain;
            cal.type = getType(row);
            calibrationPoint_t oldcal;
            oldcal.id = getId(row);
            oldcal.freq = Global::old_freq;
            oldcal.bw = Global::old_bw;
            oldcal.gain = Global::old_gain;
            oldcal.type = Global::old_type;

            QUndoCommand *editCommand = new EditCommand(ui->listView_DDCPoints,g_dcDDCContext,
                                                        row,cal,oldcal,&mtx,&lock_actions,this);
            undoStack->push(editCommand);

            drawGraph();
        }
    }
    //else qDebug() << "Invalid input data";
}
uint32_t MainWindow::insertData(biquad::Type type,int freq,double band,double gain, bool toggleSorting){
    if(toggleSorting)ui->listView_DDCPoints->setSortingEnabled(false);
    calibrationPoint_t c;
    c.id = g_dcDDCContext->GenerateId();
    c.freq = freq;
    c.bw = band;
    c.gain = gain;
    c.type = type;
    (new tableproxy(ui->listView_DDCPoints))->addRow(c);
    if(toggleSorting)ui->listView_DDCPoints->setSortingEnabled(true);
    return c.id;
}

//---View
void MainWindow::hidePoints(bool state){
    //True: shown, False: hidden
    markerPointsVisible = state;
    drawGraph();
}
void MainWindow::ScreenshotGraph(){
    QObject* obj = sender();
    if(obj == nullptr) return;

    if(obj == ui->actionScr_Magnitude_response) ui->graph->saveScreenshot();
    else if(obj == ui->actionScr_Phase_response) ui->phase_graph->saveScreenshot();
    else if(obj == ui->actionScr_Group_delay) ui->gdelay_graph->saveScreenshot();
}
void MainWindow::drawGraph(GraphType t, bool onlyUpdatePoints){
    if((t == GraphType::magnitude || t == GraphType::all) && !onlyUpdatePoints)
        ui->graph->clear();
    if((t == GraphType::groupdelay || t == GraphType::all) && !onlyUpdatePoints)
        ui->gdelay_graph->clear();
    if((t == GraphType::phase || t == GraphType::all) && !onlyUpdatePoints)
        ui->phase_graph->clear();

    if (ui->listView_DDCPoints->rowCount() <= 0)
        return;
    const int bandCount = 8192 * 2;
    std::vector<float> responseTable = g_dcDDCContext->GetMagnitudeResponseTable(bandCount, 48000.0);
    std::vector<float> phaseTable = g_dcDDCContext->GetPhaseResponseTable(bandCount, 48000.0);
    std::vector<float> gdelayTable = g_dcDDCContext->GetGroupDelayTable(bandCount, 48000.0);
    if((t == GraphType::magnitude || t == GraphType::all) && !onlyUpdatePoints) ui->graph->updatePlot(responseTable,bandCount);
    if((t == GraphType::phase || t == GraphType::all) && !onlyUpdatePoints) ui->phase_graph->updatePlot(phaseTable,bandCount);
    if((t == GraphType::groupdelay || t == GraphType::all) && !onlyUpdatePoints) ui->gdelay_graph->updatePlot(gdelayTable,bandCount);
    ui->graph->updatePoints(ui->listView_DDCPoints,markerPointsVisible);
    ui->phase_graph->updatePoints(ui->listView_DDCPoints,markerPointsVisible);
    ui->gdelay_graph->updatePoints(ui->listView_DDCPoints,markerPointsVisible);
}

//---Dialogs
void MainWindow::showIntroduction(){
    QString data = tr("Unable to open HTML file");
    QFile file(":/html/introduction.html");
    if(file.open(QIODevice::ReadOnly))
        data = file.readAll();
    file.close();
    TextPopup *t = new TextPopup(data);
    t->show();
}
void MainWindow::showHelp(){
    QString data = tr("Unable to open HTML file");
    QFile file(":/html/help.html");
    if(file.open(QIODevice::ReadOnly))
        data = file.readAll();
    file.close();
    TextPopup *t = new TextPopup(data);
    t->show();
}
void MainWindow::showKeycombos(){
    QString data = tr("Unable to open HTML file");
    QFile file(":/html/keycombos.html");
    if(file.open(QIODevice::ReadOnly))
        data = file.readAll();
    file.close();
    TextPopup *t = new TextPopup(data);
    t->show();
}
void MainWindow::showCalc(){
    calc *t = new calc();
    t->show();
}
void MainWindow::showUndoView(){
    undoView = new QUndoView(undoStack);
    undoView->setWindowTitle(tr("Undo History"));
    undoView->show();
    undoView->setAttribute(Qt::WA_QuitOnClose, false);
}

//---Import/Export
void MainWindow::importClassicVDC(){
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open VDC file"), "", tr("Viper VDC file (*.vdc)"));
    if (fileName != "" && fileName != nullptr){
        clearPoint(false);
        setActiveFile("");
        undoStack->clear();
        lock_actions = true;
        mtx.lock();

        QString data = ConversionEngine::convertVDCtoProjectFile(fileName);
        if (data.length() < 1){
            QMessageBox::warning(this,tr("Error"),tr("Cannot open file for reading"));
            lock_actions = false;
            return;
        }

        QString line;
        QTextStream stream(&data);
        while (stream.readLineInto(&line)) {
            calibrationPoint_t cal = ProjectManager::readSingleLine(line);
            if(cal.type == biquad::INVALID)
                continue;

            uint32_t id = insertData(cal.type,cal.freq,cal.bw,cal.gain,false);
            if(cal.type == biquad::CUSTOM){
                newCustomFilter(cal.custom441,cal.custom48,ui->listView_DDCPoints,ui->listView_DDCPoints->rowCount()-1);
                g_dcDDCContext->AddFilter(id,cal.custom441,cal.custom48);
            } else {
                g_dcDDCContext->AddFilter(id,cal.type,cal.freq, cal.gain, cal.bw, 48000.0,true);
            }
        }
        stream.seek(0);

        mtx.unlock();
        lock_actions = false;

        drawGraph();
        emit loadFinished();
    }
}
void MainWindow::importParametricAutoEQ(){
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Import AutoEQ config 'ParametricEQ.txt'"), "",tr( "AutoEQ ParametricEQ.txt (*ParametricEQ.txt);;All files (*.*)"));
    if (fileName != "" && fileName != nullptr){
        ui->listView_DDCPoints->clear();
        clearPoint(false);
        setActiveFile("");
        undoStack->clear();

        std::vector<calibrationPoint_t> points =
                ConversionEngine::readParametricEQFile(fileName);
        if(points.size() < 1){
            QMessageBox::warning(this,tr("Error"),tr("Unable to convert this file; no data found: %1").arg(fileName));
            return;
        }

        for(size_t i=0;i<points.size();i++){
            calibrationPoint_t cal = points.at(i);
            lock_actions = true;
            uint32_t id = insertData(biquad::Type::PEAKING,cal.freq,(double)cal.bw,(double)cal.gain);
            lock_actions = false;
            g_dcDDCContext->AddFilter(id,biquad::Type::PEAKING,cal.freq, (double)cal.gain, (double)cal.bw, 48000.0,true);

        }
        ui->listView_DDCPoints->sortItems(1,Qt::SortOrder::AscendingOrder);
        drawGraph();
        emit loadFinished();
    }
}
void MainWindow::exportVDC()
{
    mtx.lock();
    std::list<double> p1 = g_dcDDCContext->ExportCoeffs(44100.0);
    std::list<double> p2 = g_dcDDCContext->ExportCoeffs(48000.0);
    mtx.unlock();

    if (p1.empty() || p2.empty())
    {
        QMessageBox::warning(this,tr("Error"),tr("Failed to export to VDC"));
        return;
    }

#ifdef IS_WASM
    ProjectManager::exportVDC(".tmp_proj.vdc", p1, p2);
    QFile file(".tmp_proj.vdc");
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QByteArray blob = file.readAll();
    file.close();

    QFileDialog::saveFileContent(blob, currentFile.isEmpty() ? "export.vdc" : QFileInfo(currentFile).baseName() + ".vdc");
#else
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save VDC"), "", tr("VDC File (*.vdc)"));
    ProjectManager::exportVDC(fileName,p1,p2);
#endif
}
void MainWindow::exportCompatVDCProj(){
    saveAsDDCProject(true,"",true);
}
void MainWindow::downloadFromAutoEQ(){
    AutoEQSelector* sel = new AutoEQSelector();
    sel->setModal(true);
    if(sel->exec() == QDialog::Accepted){
        HeadphoneMeasurement hp = sel->getSelection();

        setActiveFile("");
        clearPoint(false);
        drawGraph();
        undoStack->clear();

        std::vector<calibrationPoint_t> points =
                ConversionEngine::readParametricEQString(hp.getParametricEQ());
        if(points.size() < 1){
            QMessageBox::warning(this,tr("Error"),tr("Invalid response.\nNo data found: %1"));
            return;
        }

        for(size_t i=0;i<points.size();i++){
            calibrationPoint_t cal = points.at(i);
            lock_actions = true;
            uint32_t id = insertData(biquad::Type::PEAKING,cal.freq,(double)cal.bw,(double)cal.gain);
            lock_actions = false;
            g_dcDDCContext->AddFilter(id,biquad::Type::PEAKING,cal.freq, (double)cal.gain, (double)cal.bw, 48000.0,true);

        }
        ui->listView_DDCPoints->sortItems(1,Qt::SortOrder::AscendingOrder);
        drawGraph();
        emit loadFinished();

    }
    sel->deleteLater();
}

//---Batch conversion
void MainWindow::batch_vdc2vdcprj(){
#ifndef IS_WASM
    QStringList filenames = QFileDialog::getOpenFileNames(this,tr("Select all VDC files to convert"),QDir::currentPath(),tr("VDC files (*.vdc)") );
    if( !filenames.isEmpty() )
    {
        QMessageBox::information(this,tr("Note"),tr(
                                     "%1 files will be converted.\nYou will now be prompted to select an output directory.").arg((int)filenames.count()));
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Output-Directory"),
                                                        "",
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

        if(dir=="")return;
        for (int l=0;l<(int)filenames.count();l++){
            QString vdcprj = ConversionEngine::convertVDCtoProjectFile(filenames.at(l));
            if (vdcprj.length() < 1){
                QMessageBox::warning(this,tr("Error"),tr("Cannot open file %1 for reading").arg(filenames.at(l)));
                continue;
            }

            QFileInfo fi(filenames.at(l));
            QString out = fi.completeBaseName();
            QFile qFile(QDir(dir).filePath(fi.completeBaseName()+".vdcprj"));
            if (qFile.open(QIODevice::WriteOnly)) {
                QTextStream out(&qFile); out << vdcprj;
                qFile.close();
            }
        }
        QMessageBox::information(this,tr("Note"),tr("Conversion finished!\nYou can find the files here:\n%1").arg(dir));
    }
#endif
}
void MainWindow::batch_parametric2vdcprj(){
#ifndef IS_WASM
    QStringList filenames = QFileDialog::getOpenFileNames(this,tr("Select all AutoEQ ParametricEQ.txt files to convert"),QDir::currentPath(),tr("AutoEQ ParametricEQ.txt (*ParametricEQ.txt);;All files (*.*)") );
    if( !filenames.isEmpty() )
    {
        QMessageBox::information(this,"Note",tr(
                                     "%1 files will be converted.\nYou will now be prompted to select an output directory.").arg(filenames.count()));
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Output-Directory"),
                                                        "",
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        if(dir=="")return;
        for (int l=0;l<(int)filenames.count();l++){
            int i = 0;
            std::vector<calibrationPoint_t> points =
                    ConversionEngine::readParametricEQFile(filenames.at(l));
            if(points.size() < 1){
                QMessageBox::warning(this,tr("Error"),tr("Unable to convert this file: %1").arg(filenames.at(l)));
                return;
            }
            QFileInfo fi(filenames.at(l));
            if(!ProjectManager::writeProjectFile(points,QDir(dir).filePath(fi.completeBaseName()+".vdcprj"),false))
                QMessageBox::warning(this,tr("Error"),tr("Cannot write file at: %1").arg(QDir(dir).filePath(fi.completeBaseName()+".vdcprj")));
        }
        QMessageBox::information(this,tr("Note"),tr("Conversion finished!\nYou can find the files here:\n%1").arg(dir));
    }
#endif
}

//---Getter
double MainWindow::getValue(DataType dat,int row){
    int i = ui->listView_DDCPoints->rowCount();
    if(i <= row)
        return 0;

    auto item = ui->listView_DDCPoints->item(row, dat);
    if(item == nullptr){
        qWarning() << "MainWindow::getValue accessed NULL table cell";
        return 0;
    }

    switch(dat){
    case type:
        return item->data(Qt::DisplayRole).toInt();
    case freq:
        return item->data(Qt::DisplayRole).toInt();
    case bw:
        return item->data(Qt::DisplayRole).toDouble();
    case gain:
        return item->data(Qt::DisplayRole).toDouble();
    }
}
biquad::Type MainWindow::getType(int row){
    QString type = ui->listView_DDCPoints->item(row,0)->data(Qt::DisplayRole).toString();
    return stringToType(type);
}
uint32_t MainWindow::getId(int row){
    return ui->listView_DDCPoints->item(row,0)->data(Qt::UserRole).toUInt();
}

//---Updater
void MainWindow::checkForUpdates()
{
#ifndef IS_WASM
    /* Apply the settings */
    m_updater->setModuleVersion (DEFS_URL, VERSION);
    m_updater->setNotifyOnFinish (DEFS_URL, true);
    m_updater->setNotifyOnUpdate (DEFS_URL, true);
    m_updater->setUseCustomAppcast (DEFS_URL, false);
    m_updater->setDownloaderEnabled (DEFS_URL, true);
    m_updater->setMandatoryUpdate (DEFS_URL, false);
    m_updater->setUseCustomInstallProcedures(DEFS_URL,true);

    /* Check for updates */
    m_updater->checkForUpdates (DEFS_URL);
#endif
}

//---Localization
void MainWindow::createLanguageMenu(void)
{
    // format systems language
    QString defaultLocale = QLocale::system().name(); // e.g. "de_DE"
    defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"
    QString m_langPath = ":/translations";
    QDir dir(m_langPath);

    // Create Language Menu to match qm translation files found
    QActionGroup* langGroup = new QActionGroup(ui->actionLanguage);
    langGroup->setExclusive(true);
    connect(langGroup, SIGNAL (triggered(QAction *)), this, SLOT (slotLanguageChanged(QAction *)));

    QMenu* submenu = ui->menuToold->addMenu(tr("Language"));

    QStringList fileNames = dir.entryList(QStringList("ddctoolbox_*.qm"));
    for (int i = 0; i < fileNames.size(); ++i) {
        // get locale extracted by filename
        QString locale;
        locale = fileNames[i]; // "TranslationExample_de.qm"

        locale.truncate(locale.lastIndexOf('.')); // "TranslationExample_de"
        locale.remove(0, locale.indexOf('_') + 1); // "de"

        QString lang = QLocale::languageToString(QLocale(locale).language());

        QAction *action = new QAction(lang, this);
        action->setCheckable(true);
        // action->setData(resourceFileName);
        action->setData(locale);

        submenu->addAction(action);
        langGroup->addAction(action);

        // set default translators and language checked
        if (defaultLocale == locale)
        {
            action->setChecked(true);
        }
    }
}
void MainWindow::slotLanguageChanged(QAction* action)
{
    if(0 == action) {
        return;
    }
    loadLanguage(action->data().toString());
}
void MainWindow::loadLanguage(const QString& rLanguage)
{
    if(m_currLang == rLanguage) {
        return;
    }
    m_currLang = rLanguage;

    QLocale locale = QLocale(m_currLang);
    QLocale::setDefault(QLocale::c());

    QString languageName = QLocale::languageToString(locale.language());

    // m_translator contains the app's translations
    QString resourceFileName = QString("%1/ddctoolbox_%2.qm").arg(":/translations").arg(rLanguage);
    switchTranslator(m_translator, resourceFileName);

}
void MainWindow::changeEvent(QEvent* event)
{
    if(0 != event) {
        switch(event->type()) {
        // this event is send if a translator is loaded
        case QEvent::LanguageChange:
            // UI will not update unless you call retranslateUi
            ui->retranslateUi(this);
            break;

            // this event is send, if the system, language changes
        case QEvent::LocaleChange:
        {
            QString locale = QLocale::system().name();
            locale.truncate(locale.lastIndexOf('_'));
            loadLanguage(locale);
        }
            break;
        default:
            break;
        }
    }
    QMainWindow::changeEvent(event);
}
void MainWindow::switchTranslator(QTranslator& translator, const QString& filename)
{
    // remove the old translator
    qApp->removeTranslator(&translator);

    // load the new translator
    bool result = translator.load(filename);

    if(!result) {
        qWarning("*** Failed translator.load(\"%s\")", filename.toLatin1().data());
        return;
    }
    qApp->installTranslator(&translator);
}

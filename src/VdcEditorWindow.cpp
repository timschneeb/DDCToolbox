#include "VdcEditorWindow.h"
#include "ui_VdcEditorWindow.h"

#include "model/FilterViewDelegate.h"
#include "model/command/AddCommand.h"
#include "model/command/InvertCommand.h"
#include "model/command/RemoveCommand.h"
#include "model/command/ShiftCommand.h"
#include "model/command/EditCommand.h"

#include "widget/AddPointDialog.h"
#include "widget/BwCalculator.h"
#include "widget/ShiftFrequencyDialog.h"
#include "widget/HtmlPopup.h"
#include "widget/StabilityReport.h"
#include "widget/AutoEqSelector.h"
#include "widget/CurveFittingDialog.h"

#include "utils/VdcProjectManager.h"
#include "item/CustomFilterListItem.h"
#include "plot/FrequencyPlot.h"

#ifdef Q_OS_WIN
#include "utils/SoftwareUpdateManager.h"
#endif

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include <widget/CsvExportDialog.h>
#include <widget/EapoExportDialog.h>


VdcEditorWindow::VdcEditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VdcEditorWindowHost), undoStack(new QUndoStack(this)),
    undoView(new QUndoView(undoStack)), bwCalc(new BwCalculator(this))
{
    Q_INIT_RESOURCE(ddceditor_resources);

    ui->setupUi(this);
    undoStack->setUndoLimit(100);
    undoView->setVisible(false);

#define DECL_UNDO_ACTION(type)\
    QAction* action##type = undoStack->create##type##Action(this, tr(#type));\
    action##type->setIcon(QPixmap(":/img/" + QString(#type).toLower() + ".svg"));\
    action##type->setShortcuts(QKeySequence::type);\
    QAction* first##type = ui->menuEdit->actions().at(0);\
    ui->menuEdit->insertAction(first##type, action##type);\
    ui->toolBar_file->addAction(action##type);

    DECL_UNDO_ACTION(Undo);
    DECL_UNDO_ACTION(Redo);
#undef DECL_UNDO_ACTION

    filterModel = new FilterModel();
    connect(filterModel, &FilterModel::dataChanged, &VdcProjectManager::instance(), &VdcProjectManager::projectModified);
    connect(filterModel, &FilterModel::dataChanged, this, &VdcEditorWindow::drawPlots);
    connect(filterModel, &FilterModel::filterEdited, [this](DeflatedBiquad previous, DeflatedBiquad current, QModelIndex index){
        EditCommand* editCmd = new EditCommand(filterModel, previous, current, index);
        undoStack->push(editCmd);
    });

    auto delegate = new FilterViewDelegate();
    delegate->setModel(filterModel);
    ui->tableView_DDCPoints->setItemDelegate(delegate);
    connect(delegate, &FilterViewDelegate::requireEditCommit, [this](EditCommand* cmd){
        undoStack->push(cmd);
    });

    VdcProjectManager::instance().initialize(filterModel);
    ui->tableView_DDCPoints->setModel(filterModel);
    ui->tableView_DDCPoints->sortByColumn(1, Qt::AscendingOrder);

    ui->tableView_DDCPoints->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(ui->tableView_DDCPoints->selectionModel(), &QItemSelectionModel::selectionChanged, [=]{updatePlots(true);});

    connect(&VdcProjectManager::instance(), &VdcProjectManager::projectClosed, [this]{ undoStack->clear(); });
    connect(&VdcProjectManager::instance(), &VdcProjectManager::projectMetaChanged, [this]{
        QString title = "DDCToolbox";
        if(VdcProjectManager::instance().currentProject().isEmpty()){
            this->setWindowTitle(title);
            return;
        }

        title += " - " + QFileInfo(VdcProjectManager::instance().currentProject()).fileName();

        if(VdcProjectManager::instance().hasUnsavedChanges())
            title += "*";
        this->setWindowTitle(title);
        emit windowTitleChanged(title);
    });

    preparePlots();
    drawPlots();

#ifndef QT_DEBUG
    ui->actionEnable_table_debug_mode->setVisible(false);
#endif

    createRecentFileActions();

    // Setup update notify bar
    ui->updateBar->setVisible(false);
#ifdef Q_OS_WINDOWS
    QLabel* updateLabel = new QLabel(ui->updateBar);
    QLabel* fakeSpacer = new QLabel(ui->updateBar);
    QPushButton* installButton = new QPushButton(ui->updateBar);
    QPushButton* changelogButton = new QPushButton(ui->updateBar);
    QPushButton* notNowButton = new QPushButton(ui->updateBar);

    fakeSpacer->setMaximumWidth(6);

    installButton->setText("Install update");
    installButton->setStyleSheet("font-weight:600;");
    installButton->setMinimumWidth(100);

    changelogButton->setText("View changelog");
    changelogButton->setMinimumWidth(100);
    notNowButton->setText("Not now");
    notNowButton->setMinimumWidth(100);

    updateLabel->setText("A new version of this application has been released. Press 'Install' to update automatically.");

    ui->updateBar->setStyleSheet(
        "QStatusBar {"
            "background-color: rgb(255, 209, 117);"
            "border: 1px solid rgb(208, 204, 201);"
        "}"
        "QStatusBar QLabel {"
            "padding: 6px;"
        "}");
    ui->updateBar->addWidget(updateLabel);
    ui->updateBar->addWidget(fakeSpacer);
    ui->updateBar->addPermanentWidget(installButton);
    ui->updateBar->addPermanentWidget(changelogButton);
    ui->updateBar->addPermanentWidget(notNowButton);
    ui->updateBar->addPermanentWidget(fakeSpacer);

    swUpdater = new SoftwareUpdateManager();
    connect(ui->actionCheck_for_updates, &QAction::triggered, swUpdater, &SoftwareUpdateManager::checkForUpdates);

    connect(notNowButton, &QAbstractButton::clicked, [this]{ ui->updateBar->setVisible(false); });
    connect(changelogButton, &QAbstractButton::clicked, swUpdater, &SoftwareUpdateManager::userRequestedChangelog);
    connect(installButton, &QAbstractButton::clicked, swUpdater, &SoftwareUpdateManager::userRequestedInstall);

    connect(swUpdater, &SoftwareUpdateManager::requestGracefulShutdown, this, &VdcEditorWindow::close);
    connect(swUpdater, &SoftwareUpdateManager::updateAvailable, [this]{ ui->updateBar->setVisible(true); });
    swUpdater->silentCheckDeferred(1000);
#else
    ui->actionCheck_for_updates->setVisible(false);
#endif

    // OS session management
    connect(qApp, &QGuiApplication::commitDataRequest,
                this, &VdcEditorWindow::commitData);

}

VdcEditorWindow::~VdcEditorWindow()
{
    delete ui;
}

void VdcEditorWindow::createRecentFileActions()
{
    QAction* recentFileAction = 0;
    for(auto i = 0; i < maxFileNr; ++i){
        recentFileAction = new QAction(this);
        recentFileAction->setVisible(false);
        QObject::connect(recentFileAction, &QAction::triggered,
                         this, [this]{
            QAction *action = qobject_cast<QAction *>(sender());
            if (action){
                VdcProjectManager::instance().closeProject();
                bool success = VdcProjectManager::instance().loadProject(action->data().toString());

                if(!success)
                    QMessageBox::critical(this, "Error", "No valid data found");
            }
        });
        recentFileActionList.append(recentFileAction);
    }

    ui->menuRecents_projects->setIcon(QIcon(QPixmap(":/img/recents.svg")));

    for(auto i = 0; i < maxFileNr; ++i)
        ui->menuRecents_projects->addAction(recentFileActionList.at(i));

    ui->menuRecents_projects->addSeparator();
    auto clearAct = new QAction(QIcon(QPixmap(":/img/clear_recents.svg")), "Clear recent projects");
    connect(clearAct, &QAction::triggered, this, [this]{
        QSettings settings;
        settings.remove("recentFiles");
        updateRecentActionList();
    });
    ui->menuRecents_projects->addAction(clearAct);

    updateRecentActionList();
}

void VdcEditorWindow::adjustForCurrentFile(const QString &filePath){
    QSettings settings;
    QStringList recentFilePaths =
            settings.value("recentFiles").toStringList();
    recentFilePaths.removeAll(filePath);
    recentFilePaths.prepend(filePath);
    while (recentFilePaths.size() > maxFileNr)
        recentFilePaths.removeLast();
    settings.setValue("recentFiles", recentFilePaths);

    updateRecentActionList();
}

void VdcEditorWindow::updateRecentActionList(){
    QSettings settings;
    QStringList recentFilePaths =
            settings.value("recentFiles").toStringList();

    auto itEnd = 0;
    if(recentFilePaths.size() <= maxFileNr)
        itEnd = recentFilePaths.size();
    else
        itEnd = maxFileNr;

    for (auto i = 0; i < itEnd; ++i) {
        QString strippedName = QFileInfo(recentFilePaths.at(i)).fileName();
        recentFileActionList.at(i)->setText(strippedName);
        recentFileActionList.at(i)->setData(recentFilePaths.at(i));
        recentFileActionList.at(i)->setVisible(true);
    }

    ui->menuRecents_projects->setEnabled(itEnd > 0);

    for (auto i = itEnd; i < maxFileNr; ++i)
        recentFileActionList.at(i)->setVisible(false);
}

void VdcEditorWindow::closeEvent(QCloseEvent *ev)
{
    if(VdcProjectManager::instance().hasUnsavedChanges() && filterModel->rowCount() > 0){
        int ret = QMessageBox::warning(
                    this,
                    tr("DDCToolbox"),
                    tr("The document has been modified.\nDo you want to save your changes?"),
                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        switch (ret) {
        case QMessageBox::Save:
            saveProject();
            break;
        case QMessageBox::Discard:
            break;
        case QMessageBox::Cancel:
        default:
            ev->ignore();
            return;
        }
    }
    ev->accept();
    QMainWindow::closeEvent(ev);
}

void VdcEditorWindow::setOrientation(Qt::Orientation orientation){
    ui->splitter->setOrientation(orientation);
}

void VdcEditorWindow::commitData(QSessionManager& manager)
{
    if (VdcProjectManager::instance().hasUnsavedChanges() &&
        filterModel->rowCount() > 0)
    {
        if(manager.allowsInteraction()){
            int ret = QMessageBox::warning(
                        this,
                        tr("DDCToolbox"),
                        tr("The document has been modified and your operating system has sent a shutdown request.\n"
                           "Do you want to save your changes?"),
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

            switch (ret) {
            case QMessageBox::Save:
                saveProject();
                manager.release();
                break;
            case QMessageBox::Discard:
                break;
            case QMessageBox::Cancel:
            default:
                manager.cancel();
                return;
            }
        }
        else
        {
            // No interaction allowed. Attempt to save project.
            QString path = VdcProjectManager::instance().currentProject();
            if (path.isEmpty())
                return;

            VdcProjectManager::instance().saveProject(path);
        }
    }
}

void VdcEditorWindow::saveProject()
{
    QString path = VdcProjectManager::instance().currentProject();
    if(path.isEmpty() || sender() == ui->actionSaveAs){
        path = QFileDialog::getSaveFileName(this, "Save VDC Project File",
                                            "", "ViPER DDC Project (*.vdcprj)");
    }

    if (path.isEmpty())
        return;

    VdcProjectManager::instance().saveProject(path);
    adjustForCurrentFile(path);
}

void VdcEditorWindow::loadProject()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open VDC Project File"),
                                                "", tr("ViPER DDC Project (*.vdcprj)"));
    if(file.isEmpty())
        return;

    VdcProjectManager::instance().closeProject();
    bool success = VdcProjectManager::instance().loadProject(file);

    if(!success)
        QMessageBox::critical(this, "Error", "No valid data found");

    adjustForCurrentFile(file);
}

void VdcEditorWindow::closeProject(){
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "DDC Toolbox", tr("Are you sure? All unsaved changes will be lost."),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
        VdcProjectManager::instance().closeProject();
}

void VdcEditorWindow::exportProject()
{
    std::list<double> p1 = filterModel->exportCoeffs(44100.0);
    std::list<double> p2 = filterModel->exportCoeffs(48000.0);

    if (p1.empty() || p2.empty())
    {
        QMessageBox::warning(this,tr("Error"),tr("Your current project is empty and contains no filters"));
        return;
    }

    auto s = new StabilityReport(filterModel, this);
    if(!s->isReportPositive()){
        s->exec();
        delete s;
        return;
    }
    delete s;

    if(sender() == ui->actionVDC)
    {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save VDC"), "", tr("VDC File (*.vdc)"));
        VdcProjectManager::instance().exportProject(fileName, p1, p2);
    }
    else if(sender() == ui->actionEqualizerAPO_configuration)
    {
        auto srDlg = new EapoExportDialog(this);
        if(!srDlg->exec()){
            return;
        }

        std::list<double> p = filterModel->exportCoeffs(srDlg->getResult(), true);

#ifdef Q_OS_WINDOWS
        QString dir = "C:\\Program Files\\EqualizerAPO\\config";
#else
        QString dir = "";
#endif

        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save EqualizerAPO config"), dir, tr("Equalizer APO config file (*.txt)"));
        VdcProjectManager::instance().exportEapoConfig(fileName, p, srDlg->getResult());
        srDlg->deleteLater();
    }
    else if(sender() == ui->actionCSV_dataset)
    {
        auto dlg = new CsvExportDialog(this);
        if(!dlg->exec()){
            return;
        }

        std::list<double> p = filterModel->exportCoeffs(dlg->samplerate(), true);

        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save CSV dataset"), "", tr("CSV dataset (*.csv)"));
        VdcProjectManager::instance().exportCsv(fileName, p, dlg->delimiter(), dlg->format(), dlg->numFormat(), dlg->includeHeader());
        dlg->deleteLater();
    }

}

void VdcEditorWindow::curveFitting()
{
    auto dlg = new CurveFittingDialog(this);
    int ret = dlg->exec();
    if(ret && dlg->getResults().count() > 0){
        VdcProjectManager::instance().closeProject();
        filterModel->appendAllDeflated(dlg->getResults());
    }
    else if(ret && dlg->getResults().count() <= 0){
        QMessageBox::critical(this, "Error", "Failed to process data. Algorithm returned an empty response or was terminated.");
    }

    dlg->deleteLater();
}

/* ---- Editor interactions ----*/
void VdcEditorWindow::addPoint(){
    AddPointDialog *dlg = new AddPointDialog(this);

    if(dlg->exec()){
        Biquad* b = dlg->getBiquad();
        QUndoCommand *addCommand = new AddCommand(filterModel, b);
        undoStack->push(addCommand);
    }

    dlg->deleteLater();
}

void VdcEditorWindow::removePoint(){
    QItemSelectionModel* select = ui->tableView_DDCPoints->selectionModel();
    
    if (select->hasSelection())
    {
        QUndoCommand *removeCommand = new RemoveCommand(filterModel, select->selectedRows());
        undoStack->push(removeCommand);
    }
}

void VdcEditorWindow::clearAll(){
    QUndoCommand *removeCommand = new RemoveCommand(filterModel);
    undoStack->push(removeCommand);
}

void VdcEditorWindow::invertSelection(){
    QItemSelectionModel* select = ui->tableView_DDCPoints->selectionModel();

    if(!select->hasSelection()){
        QMessageBox::information(this, tr("Invert selection"), tr("No rows selected"));
        return;
    }

    QUndoCommand *invertCommand = new InvertCommand(filterModel,
                                                    select->selectedRows());
    undoStack->push(invertCommand);
}

void VdcEditorWindow::shiftSelection(){
    QItemSelectionModel* select = ui->tableView_DDCPoints->selectionModel();

    if(!select->hasSelection()){
        QMessageBox::information(this, tr("Shift selection"), tr("No rows selected"));
        return;
    }

    ShiftFrequencyDialog* sf = new ShiftFrequencyDialog(filterModel, this);

    if(sf->exec()){
        QUndoCommand *shiftCommand =
                new ShiftCommand(filterModel, select->selectedRows(), sf->getResult());
        undoStack->push(shiftCommand);
    }

    sf->deleteLater();
}

void VdcEditorWindow::checkStability(){
    auto s = new StabilityReport(filterModel, this);

    if(s->isReportPositive())
        QMessageBox::information(this, "Stability check", "All filters appear to be stable");
    else
        s->exec();
    s->deleteLater();
}

/* ---- View ----*/
void VdcEditorWindow::showPointsAlways(bool state){
    markerPointsVisible = state;
    drawPlots();
}

void VdcEditorWindow::savePlotScreenshot(){
    QObject* obj = sender();
    if(obj == nullptr) return;
    
    if(obj == ui->actionScr_Magnitude_response) ui->graph->saveScreenshot();
    else if(obj == ui->actionScr_Phase_response) ui->phase_graph->saveScreenshot();
    else if(obj == ui->actionScr_Group_delay) ui->gdelay_graph->saveScreenshot();
}

void VdcEditorWindow::updatePlots(bool onlyUpdatePoints){
    if(!onlyUpdatePoints)
    {
        const int bandCount = 8192 * 2;
        QVector<float> responseTable = filterModel->getMagnitudeResponseTable(bandCount, 48000.0);
        QVector<float> phaseTable = filterModel->getPhaseResponseTable(bandCount, 48000.0);
        QVector<float> gdelayTable = filterModel->getGroupDelayTable(bandCount, 48000.0);
        ui->graph->clearAll();
        ui->gdelay_graph->clearAll();
        ui->phase_graph->clearAll();
        ui->graph->updatePlot(responseTable,bandCount);
        ui->phase_graph->updatePlot(phaseTable,bandCount);
        ui->gdelay_graph->updatePlot(gdelayTable,bandCount);
    }

    ui->graph->setAllMarkerPointsVisible(markerPointsVisible);
    ui->phase_graph->setAllMarkerPointsVisible(markerPointsVisible);
    ui->gdelay_graph->setAllMarkerPointsVisible(markerPointsVisible);
    ui->graph->updatePoints();
    ui->phase_graph->updatePoints();
    ui->gdelay_graph->updatePoints();
}

void VdcEditorWindow::setDebugMode(bool state){
    filterModel->setDebugMode(state);
    ui->tableView_DDCPoints->repaint();
}

/* ---- Dialogues ----*/
void VdcEditorWindow::showHelp(){
    QString res;

    if(sender() == nullptr) return;
    else if(sender() == ui->actionIntroduction) res = ":/html/introduction.html";
    else if(sender() == ui->actionKey_shortcuts) res = ":/html/keycombos.html";

    QFile file(res);
    QString data;
    if(file.open(QIODevice::ReadOnly))
        data = file.readAll();
    else
        data = "Unable to open HTML file";
    file.close();

    HtmlPopup *t = new HtmlPopup(data, this);
    t->setModal(true);
    t->exec();
    t->deleteLater();
}

void VdcEditorWindow::showCalc(){
    bwCalc->show();
    bwCalc->raise();
    bwCalc->activateWindow();
}

void VdcEditorWindow::showUndoView(){
    undoView->setWindowTitle(tr("Undo History"));
    undoView->show();
    undoView->setAttribute(Qt::WA_QuitOnClose, false);
}

/* ---- Conversion ----*/
void VdcEditorWindow::importClassicVdc(){

    QString file = QFileDialog::getOpenFileName(this, tr("Open classic VDC file"),
                                                "", tr("Viper VDC file (*.vdc)"));
    if(file.isEmpty())
        return;

    VdcProjectManager::instance().closeProject();
    bool success = VdcProjectManager::instance().loadVdc(file);

    if(!success)
        QMessageBox::critical(this, "Error", "No valid data found\nKeep in mind that only classic VDC files with 'Peaking filters' are supported.");
}

void VdcEditorWindow::importParametricAutoEQ(){
    QString file = QFileDialog::getOpenFileName(this, tr("Import AutoEQ config 'ParametricEQ.txt'"),
                                                "", tr( "AutoEQ ParametricEQ.txt (*ParametricEQ.txt);;All files (*.*)"));
    if(file.isEmpty())
        return;

    VdcProjectManager::instance().closeProject();
    bool success = VdcProjectManager::instance().loadParametricEq(file);

    if(!success)
        QMessageBox::critical(this, "Error", "No valid data found\nConversion failed");
}

void VdcEditorWindow::downloadFromAutoEQ(){
    AutoEQSelector* sel = new AutoEQSelector();
    sel->setModal(true);
    if(sel->exec() == QDialog::Accepted){
        HeadphoneMeasurement hp = sel->getSelection();
        
        VdcProjectManager::instance().closeProject();
        bool success = VdcProjectManager::instance().loadParametricEqString(hp.getParametricEQ());
        if(!success)
            QMessageBox::critical(this, "Error", "No valid data found\nConversion failed");
    }
    sel->deleteLater();
}

void VdcEditorWindow::batchConvert(){
    bool isVdcConversion = sender() == ui->actionBatchVdc;
    QString requestedType = isVdcConversion ?
                "VDC files (*.vdc)" : "AutoEQ ParametricEQ.txt (*ParametricEQ.txt);;All files (*.*)";


    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select all files to convert"),
                                                      QDir::currentPath(), requestedType );
    if( !files.isEmpty() )
    {
        QMessageBox::information(this,tr("Note"),
                                 tr("%1 files will be converted.\nYou will now be prompted to select an output directory.").arg((files.count())));
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select output directory"),
                                                        "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(dir.isEmpty())
            return;

        for (const auto& file : files){
            QVector<DeflatedBiquad> biquads;

            if(isVdcConversion)
                biquads = VdcProjectManager::readVdc(file);
            else
                biquads = VdcProjectManager::readParametricEq(file);

            VdcProjectManager::writeProject(QDir(dir).filePath(QFileInfo(file).completeBaseName() + ".vdcprj"),
                                            biquads);
        }
        QMessageBox::information(this,tr("Note"),tr("Conversion finished!\nYou can find the files here:\n%1").arg(dir));
    }
}

/* ---- Preparation ---- */
void VdcEditorWindow::preparePlots(){
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

    ui->graph->setModel(ui->tableView_DDCPoints, filterModel);
    ui->gdelay_graph->setModel(ui->tableView_DDCPoints, filterModel);
    ui->phase_graph->setModel(ui->tableView_DDCPoints, filterModel);

    ui->graph->setMode(FrequencyPlot::PlotType::magnitude, this);
    ui->gdelay_graph->setMode(FrequencyPlot::PlotType::group_delay, this);
    ui->phase_graph->setMode(FrequencyPlot::PlotType::phase_response, this);
}

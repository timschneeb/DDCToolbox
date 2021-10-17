#include "CurveFittingDialog.h"
#include "CurveFittingWorkerDialog.h"
#include "ui_CurveFittingDialog.h"

#include "CurveFittingWorkerDialog.h"
#include "Expander.h"
#include "platform/OSXHtmlSizingPatch.h"
#include "utils/CSVParser.h"
#include "utils/CurveFittingWorker.h"
#include "utils/QInt64Validator.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

CurveFittingDialog::CurveFittingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveFittingDialog)
{
    ui->setupUi(this);

    // Connect signals
    connect(ui->fileSelection, &QAbstractButton::clicked, this, &CurveFittingDialog::selectFile);
    connect(ui->projectLink, &QAbstractButton::clicked, this, &CurveFittingDialog::visitProject);

    // Rearrange layout and insert expander
    auto * rngLayout = new QVBoxLayout(this);
    rngLayout->setContentsMargins(6, 0, 0, 0);
    rngLayout->addWidget(ui->widget);
    auto * fgridLayout = new QVBoxLayout(this);
    fgridLayout->setContentsMargins(6, 0, 0, 0);
    fgridLayout->addWidget(ui->fgrid_container);

    auto * deLayout = new QVBoxLayout(this);
    deLayout->setContentsMargins(6, 0, 0, 0);
    deLayout->addWidget(ui->algo_de_container);
    auto * chioLayout = new QVBoxLayout(this);
    chioLayout->setContentsMargins(6, 0, 0, 0);
    chioLayout->addWidget(ui->algo_chio_container);
    auto * sgdLayout = new QVBoxLayout(this);
    sgdLayout->setContentsMargins(6, 0, 0, 0);
    sgdLayout->addWidget(ui->algo_sgd_container);

    algo_de = new Expander("Differential evolution options", 300, ui->mainPane);
    algo_de->setContentLayout(*deLayout);
    algo_chio = new Expander("CHIO options", 300, ui->mainPane);
    algo_chio->setContentLayout(*chioLayout);
    algo_sgd = new Expander("SGD options", 300, ui->mainPane);
    algo_sgd->setContentLayout(*sgdLayout);
    fgrid = new Expander("Axis rebuilding", 300, ui->mainPane);
    fgrid->setContentLayout(*fgridLayout);
    advanced_rng = new Expander("Randomness options", 300, ui->mainPane);
    advanced_rng->setContentLayout(*rngLayout);

    ui->mainPane->layout()->addWidget(algo_sgd);
    ui->mainPane->layout()->addWidget(algo_de);
    ui->mainPane->layout()->addWidget(algo_chio);
    ui->mainPane->layout()->addWidget(fgrid);
    ui->mainPane->layout()->addWidget(advanced_rng);
    ui->mainPane->layout()->addWidget(ui->previewSwitchLayout);
    ui->mainPane->layout()->addWidget(ui->footer);

    QList<Expander*> _expanders(std::initializer_list<Expander*>({algo_de, algo_chio, algo_sgd, fgrid, advanced_rng}));
    for(const auto& exp : qAsConst(_expanders)){
        connect(exp, &Expander::stateChanged, this, [=](bool state){
            if(state){
                for(const auto& exp : _expanders){
                    if(exp != sender()){
                        exp->setState(false);
                    }
                }
            }
        });
    }

    // Prepare seed
    ui->adv_random_seed->setValidator(new QInt64Validator(0, UINT64_MAX, ui->adv_random_seed));
    ui->adv_random_seed->setText(QString::number(((uint64_t)rand() << 32ull) | rand()));

    // Setup UI
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setText("Calculate");
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CurveFittingDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &CurveFittingDialog::reject);

    connect(ui->algorithmType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CurveFittingDialog::updateSupportedProperties);
    ui->status_panel->setVisible(false);

    // Workaround to avoid glitched out window height sizing
    this->setGeometry(this->geometry().x(), this->geometry().y(), this->width(), 460);
    this->move(parentWidget()->window()->frameGeometry().topLeft() +
               parentWidget()->window()->rect().center() - rect().center());

    updateSupportedProperties(ui->algorithmType->currentIndex());

    // Prepare preview plot
    connect(ui->fgrid_avgbw, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CurveFittingDialog::updatePreviewPlot);
    connect(ui->fgrid_force_convert, &QCheckBox::toggled, this, &CurveFittingDialog::updatePreviewPlot);
    connect(ui->invert_gain, &QCheckBox::toggled, this, &CurveFittingDialog::updatePreviewPlot);
    connect(ui->previewToggle, &QPushButton::toggled, this, &CurveFittingDialog::setWindowExpanded);
    setWindowExpanded(false);

    ui->previewPlot->yAxis->setRange(-10, -10);
    ui->previewPlot->yAxis->setLabel("Target (dB)");

    ui->previewPlot->xAxis->setRange(QCPRange(20, 24000));
    ui->previewPlot->xAxis->setLabel("Frequency (Hz)");

    ui->previewPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->previewPlot->legend->setFont(legendFont);
    ui->previewPlot->legend->setSelectedFont(legendFont);
    ui->previewPlot->legend->setSelectableParts(QCPLegend::spItems);

    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    ui->previewPlot->xAxis->setTicker(logTicker);
    ui->previewPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->previewPlot->rescaleAxes();

    ui->previewPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->previewPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
    ui->previewPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)(Qt::AlignBottom|Qt::AlignRight));
    ui->previewPlot->replot();

    // Prepare fgrid
    connect(ui->fgrid_force_convert, &QCheckBox::toggled, ui->fgrid_avgbw, &QCheckBox::setEnabled);
    ui->fgrid_force_convert->setChecked(false);
    ui->fgrid_avgbw->setEnabled(false);
}

CurveFittingDialog::~CurveFittingDialog()
{
    delete ui;
}

void CurveFittingDialog::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->previewPlot->legend->selectTest(pos, false) >= 0)
    {
        menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
        menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
        menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
        menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
        menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
    }
    menu->popup(ui->previewPlot->mapToGlobal(pos));
}

void CurveFittingDialog::moveLegend()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender()))
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);
        if (ok)
        {
            ui->previewPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
            ui->previewPlot->replot();
        }
    }
}

void CurveFittingDialog::visitProject()
{
    QDesktopServices::openUrl(QUrl("https://github.com/james34602/libMultivariateOpt"));
}

void CurveFittingDialog::selectFile()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open CSV dataset"),
                                                "", tr("CSV dataset (*.csv)"));
    if(file.isEmpty())
        return;

    ui->filePath->setText(file);
    parseCsv();
}

void CurveFittingDialog::parseCsv(){
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    freq.clear();
    gain.clear();

    QString path = ui->filePath->text();

    if(!QFile::exists(path)){
        QMessageBox::warning(this, "CSV parser", "Selected file does not exist. Please choose another one.");
        setStatus(false, "File not found");
        return;
    }

    std::ifstream file(path.toStdString());
    for(const auto& row: CSVRange(file))
    {
        double freq_val, gain_val;

        if(row.size() < 1){
            // Row does not contain enough columns, skip it
            continue;
        }

        try {
            // Check if field contains numeric content, otherwise skip row
            freq_val = std::stod(std::string(row[0]));
            gain_val = std::stod(std::string(row[1]));
        } catch (std::invalid_argument&) {
            // Row does not contain expected data, skip it
            continue;
        }

        freq.push_back(freq_val);
        gain.push_back(gain_val);
    }

    if(freq.empty() && gain.empty()){
        setStatus(false, "No valid rows found");
        return;
    }

    /* Update optimization boundary gain */
    uint size = freq.size();
    double* flt_freqList = (double*)malloc(size * sizeof(double));
    double* targetList = (double*)malloc(size * sizeof(double));
    memcpy(flt_freqList, freq.constData(), size * sizeof(double));
    memcpy(targetList, gain.constData(), size * sizeof(double));

    bool is_nonuniform = false;

    CurveFittingWorker::preprocess(flt_freqList,
                                   targetList,
                                   size,
                                   44100,
                                   ui->fgrid_force_convert->isChecked(),
                                   ui->fgrid_avgbw->value(),
                                   &is_nonuniform,
                                   ui->invert_gain->isChecked());
    ui->fgrid_axis_linearity->setText(is_nonuniform ? "Non-uniform grid" : "Uniform grid");

    ui->fgrid_force_convert->setChecked(!is_nonuniform);
    ui->fgrid_avgbw->setEnabled(!is_nonuniform);

    free(flt_freqList);
    free(targetList);

    setStatus(true, QString::number(freq.size()) + " rows loaded");
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);

    if(ui->previewPlotContainer->isVisible()){
        updatePreviewPlot();
    }
}

void CurveFittingDialog::setStatus(bool success, const QString& text){
    ui->status_panel->setVisible(true);

    if(success)
        ui->status_img->setPixmap(QPixmap(":/img/success.svg"));
    else
        ui->status_img->setPixmap(QPixmap(":/img/critical.svg"));

    ui->status_label->setText(text);
}

QVector<DeflatedBiquad> CurveFittingDialog::getResults() const
{
    return results;
}

DoubleRange CurveFittingDialog::calculateYAxisRange(bool exportDataset, double** ret_freq, double** ret_gain){
    uint size = freq.size();

    if(size < 1){
        if(exportDataset)
        {
            ret_freq = nullptr;
            ret_gain = nullptr;
        }
        return DoubleRange(-40, 40);
    }

    double* flt_freqList = (double*)malloc(size * sizeof(double));
    double* targetList = (double*)malloc(size * sizeof(double));
    memcpy(flt_freqList, freq.constData(), size * sizeof(double));
    memcpy(targetList, gain.constData(), size * sizeof(double));

    CurveFittingWorker::preprocess(flt_freqList,
                                   targetList,
                                   size,
                                   44100,
                                   ui->fgrid_force_convert->isChecked(),
                                   ui->fgrid_avgbw->value(),
                                   nullptr,
                                   ui->invert_gain->isChecked());

    double lowGain = targetList[0];
    double upGain = targetList[0];
    for (uint i = 1; i < size; i++)
    {
        if (targetList[i] < lowGain)
            lowGain = targetList[i];
        if (targetList[i] > upGain)
            upGain = targetList[i];
    }
    lowGain -= 32.0;
    upGain += 32.0;

    if(exportDataset)
    {
        *ret_freq = flt_freqList;
        *ret_gain = targetList;
    }
    else
    {
        free(flt_freqList);
        free(targetList);
    }

    return DoubleRange(lowGain, upGain);
}

void CurveFittingDialog::updatePreviewPlot(){
    uint size = freq.size();

    if(size < 1){
        return;
    }

    double* flt_freqList = nullptr;
    double* targetList = nullptr;
    DoubleRange range = calculateYAxisRange(true, &flt_freqList, &targetList);

    ui->previewPlot->clearGraphs();

    if(flt_freqList == nullptr || targetList == nullptr)
    {
        return;
    }

    ui->previewPlot->yAxis->setRange(range.first, range.second);

    auto *pGraphOrig = ui->previewPlot->addGraph();
    QPen graphPen;
    graphPen.setColor(QColor(60, 60, 60));
    pGraphOrig->setPen(graphPen);
    pGraphOrig->setName("Original curve");
    pGraphOrig->setAdaptiveSampling(true);
    for(int i = 0; i < freq.size(); i++){
        pGraphOrig->addData(freq.constData()[i], (double)gain.constData()[i]);
    }

    auto *pGraph0 = ui->previewPlot->addGraph();
    pGraph0->setName("Processed curve");
    pGraph0->setAdaptiveSampling(true);

    for(uint i = 0; i < size; i++){
        pGraph0->addData(flt_freqList[i], (double)targetList[i]);
    }

    ui->previewPlot->replot(QCustomPlot::rpQueuedReplot);

    free(flt_freqList);
    free(targetList);
}

void CurveFittingDialog::setWindowExpanded(bool b){
    this->setMinimumWidth(b ? (385 + 515) : 385);
    this->setMaximumWidth(b ? (385 + 515) : 385);
    ui->previewPlotContainer->setVisible(b);
    ui->previewToggle->setText(b ? "Preview <<" : "Preview >>");

    if(b){
        updatePreviewPlot();
    }
}

void CurveFittingDialog::accept()
{
    this->hide();

    CurveFittingOptions options((CurveFittingOptions::AlgorithmType) ui->algorithmType->currentIndex(),
                                freq.data(),
                                gain.data(),
                                freq.count(),
                                ui->adv_random_seed->text().toLong(),
                                (CurveFittingOptions::ProbDensityFunc) ui->adv_prob_density_func->currentIndex(),
                                ui->fgrid_force_convert->isChecked(),
                                ui->iterations->value(),
                                ui->iterations_b->value(),
                                ui->iterations_c->value(),
                                ui->fgrid_avgbw->value(),
                                ui->rnd_pop_k->value(), ui->rnd_pop_n->value(),
                                ui->algo_de_probibound->value(),
                                ui->algo_chio_maxsolsurvive->value(), ui->algo_chio_c0->value(), ui->algo_chio_spreadingrate->value(),
                                ui->invert_gain->isChecked(),
                                ui->modelComplex->value(),
                                ui->sgd_lr_1->value(), ui->sgd_ldr_1->value(), ui->sgd_lr_2->value(), ui->sgd_ldr_2->value());

    auto *worker = new CurveFittingWorkerDialog(options, calculateYAxisRange(false, NULL, NULL), this);

    // Launch worker dialog and halt until finished or cancelled
    if(!worker->exec()){
        reject();
        return;
    }
    results = worker->getResults();
    worker->deleteLater();

    /* These vectors must not be cleared or modified while the worker is active.
     * The internal C array of QVector<float> is directly referenced  */
    freq.clear();
    gain.clear();

    QDialog::accept();
}

void CurveFittingDialog::updateSupportedProperties(int index){

    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_SGD:
        ui->iterations->setValue(5000);
        ui->iterations_b->setValue(50000);
        break;
    case CurveFittingOptions::AT_HYBRID_SGD_DE:
        ui->iterations->setValue(5000);
        ui->iterations_b->setValue(50000);
        ui->iterations_c->setValue(20000);
        break;
    case CurveFittingOptions::AT_HYBRID_SGD_CHIO:
        ui->iterations->setValue(5000);
        ui->iterations_b->setValue(50000);
        ui->iterations_c->setValue(4000);
        break;
    case CurveFittingOptions::AT_DIFF_EVOLUTION:
        ui->iterations->setValue(20000);
        break;
    case CurveFittingOptions::AT_CHIO:
        ui->iterations->setValue(4000);
        break;
    }

    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_DIFF_EVOLUTION:
    case CurveFittingOptions::AT_HYBRID_SGD_DE:
        algo_de->setVisible(true);
        break;
    default:
        algo_de->setVisible(false);
        break;
    }

    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_HYBRID_SGD_CHIO:
    case CurveFittingOptions::AT_CHIO:
        algo_chio->setVisible(true);
        break;
    default:
        algo_chio->setVisible(false);
        break;
    }

    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_SGD:
    case CurveFittingOptions::AT_HYBRID_SGD_CHIO:
    case CurveFittingOptions::AT_HYBRID_SGD_DE:
        algo_sgd->setVisible(true);
        break;
    default:
        algo_sgd->setVisible(false);
        break;
    }

    QString shortcutA("");
    QString shortcutB("");
    QString shortcutC("");
    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_SGD:
        shortcutA = "sgd1";
        shortcutB = "sgd2";
        break;
    case CurveFittingOptions::AT_HYBRID_SGD_DE:
        shortcutA = "sgd1";
        shortcutB = "sgd2";
        shortcutC = "de";
        break;
    case CurveFittingOptions::AT_HYBRID_SGD_CHIO:
        shortcutA = "sgd1";
        shortcutB = "sgd2";
        shortcutC = "chio";
        break;
    default:
        break;
    }

    QString htmlA = "Iterations<span style='font-size:11pt; vertical-align:sub;'> " + shortcutA + "</span>";
    QString htmlB = "Iterations<span style='font-size:11pt; vertical-align:sub;'> " + shortcutB + "</span>";
    QString htmlC = "Iterations<span style='font-size:11pt; vertical-align:sub;'> " + shortcutC + "</span>";

#ifdef __APPLE__
    htmlA = OSXHtmlSizingPatch::patchTextSize(htmlA);
    htmlB = OSXHtmlSizingPatch::patchTextSize(htmlB);
    htmlC = OSXHtmlSizingPatch::patchTextSize(htmlC);
#endif

    ui->iteration_label_a->setText(htmlA);
    ui->iteration_label_b->setText(htmlB);
    ui->iteration_label_c->setText(htmlC);

    if(shortcutA.length() <= 0)
    {
        ui->iteration_label_a->setText("Iterations");
    }

    bool visibleB = shortcutB.length() > 0;
    ui->iteration_label_b->setVisible(visibleB);
    ui->iteration_content_b->setVisible(visibleB);

    if(visibleB)
    {
        ui->formLayout->insertRow(5, ui->iteration_label_b, ui->iteration_content_b);
    }
    else
    {
        ui->formLayout->removeWidget(ui->iteration_label_b);
        ui->formLayout->removeWidget(ui->iteration_content_b);
    }

    bool visibleC = shortcutC.length() > 0;
    ui->iteration_label_c->setVisible(visibleC);
    ui->iteration_content_c->setVisible(visibleC);

    if(visibleC)
    {
        ui->formLayout->insertRow(7, ui->iteration_label_c, ui->iteration_content_c);
    }
    else
    {
        ui->formLayout->removeWidget(ui->iteration_label_c);
        ui->formLayout->removeWidget(ui->iteration_content_c);
    }
}

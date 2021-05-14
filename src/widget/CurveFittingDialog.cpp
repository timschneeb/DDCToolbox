#include "CurveFittingDialog.h"
#include "CurveFittingWorkerDialog.h"
#include "ui_CurveFittingDialog.h"

#include "Expander.h"
#include "utils/CSVParser.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

#include <utils/CurveFittingWorker.h>
#include <utils/QInt64Validator.h>

#include <platform/OSXHtmlSizingPatch.h>

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
    auto * optLayout = new QVBoxLayout(this);
    optLayout->setContentsMargins(6, 0, 0, 0);
    optLayout->addWidget(ui->obc_container);
    auto * fgridLayout = new QVBoxLayout(this);
    fgridLayout->setContentsMargins(6, 0, 0, 0);
    fgridLayout->addWidget(ui->fgrid_container);

    auto * deLayout = new QVBoxLayout(this);
    deLayout->setContentsMargins(6, 0, 0, 0);
    deLayout->addWidget(ui->algo_de_container);
    auto * flowerLayout = new QVBoxLayout(this);
    flowerLayout->setContentsMargins(6, 0, 0, 0);
    flowerLayout->addWidget(ui->algo_flower_container);
    auto * chioLayout = new QVBoxLayout(this);
    chioLayout->setContentsMargins(6, 0, 0, 0);
    chioLayout->addWidget(ui->algo_chio_container);
    auto * fminLayout = new QVBoxLayout(this);
    fminLayout->setContentsMargins(6, 0, 0, 0);
    fminLayout->addWidget(ui->algo_fmin_container);

    algo_de = new Expander("Differential evolution options", 300, ui->mainPane);
    algo_de->setContentLayout(*deLayout);
    algo_flower = new Expander("Flower pollination search options", 300, ui->mainPane);
    algo_flower->setContentLayout(*flowerLayout);
    algo_chio = new Expander("CHIO options", 300, ui->mainPane);
    algo_chio->setContentLayout(*chioLayout);
    algo_fmin = new Expander("Bounded simplex search options", 300, ui->mainPane);
    algo_fmin->setContentLayout(*fminLayout);
    opt_boundary_constraints = new Expander("Optimization boundary constraints", 300, ui->mainPane);
    opt_boundary_constraints->setContentLayout(*optLayout);
    fgrid = new Expander("Axis rebuilding", 300, ui->mainPane);
    fgrid->setContentLayout(*fgridLayout);
    advanced_rng = new Expander("Randomness options", 300, ui->mainPane);
    advanced_rng->setContentLayout(*rngLayout);

    ui->mainPane->layout()->addWidget(algo_de);
    ui->mainPane->layout()->addWidget(algo_flower);
    ui->mainPane->layout()->addWidget(algo_chio);
    ui->mainPane->layout()->addWidget(algo_fmin);
    ui->mainPane->layout()->addWidget(opt_boundary_constraints);
    ui->mainPane->layout()->addWidget(fgrid);
    ui->mainPane->layout()->addWidget(advanced_rng);
    ui->mainPane->layout()->addWidget(ui->previewSwitchLayout);
    ui->mainPane->layout()->addWidget(ui->footer);

    QList<Expander*> _expanders(std::initializer_list<Expander*>({algo_de, algo_flower, algo_chio, algo_fmin, opt_boundary_constraints, fgrid, advanced_rng}));
    for(const auto& exp : _expanders){
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

    /* Update optimization boundary freq */
    const double fs = 44100; // <- Don't forget to update this
    ui->obc_freq_max->setValue(fs / 2 - 1);

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
    connect(ui->previewToggle, &QPushButton::toggled, this, &CurveFittingDialog::setWindowExpanded);
    setWindowExpanded(false);

    ui->previewPlot->yAxis->setRange(-10, -10);
    ui->previewPlot->yAxis->setLabel("Target");

    ui->previewPlot->xAxis->setRange(QCPRange(20, 24000));
    ui->previewPlot->xAxis->setLabel("Frequency");

    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    ui->previewPlot->xAxis->setTicker(logTicker);
    ui->previewPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->previewPlot->rescaleAxes();
}

CurveFittingDialog::~CurveFittingDialog()
{
    delete ui;
}

void CurveFittingDialog::visitProject()
{
    QDesktopServices::openUrl(QUrl("https://github.com/james34602/libgradfreeOpt"));
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
    for(auto& row: CSVRange(file))
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
        } catch (std::invalid_argument &ex) {
            // Row does not contain expected data, skip it
            continue;
        }

        freq.push_back(freq_val);
        gain.push_back(gain_val);
    }

    if(freq.size() < 1 && gain.size() < 1){
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
                                   &is_nonuniform);
    ui->fgrid_axis_linearity->setText(is_nonuniform ? "Non-uniform grid" : "Uniform grid");

    double lowGain = targetList[0];
    double upGain = targetList[0];
    for (uint i = 1; i < size; i++)
    {
        if (targetList[i] < lowGain)
            lowGain = targetList[i];
        if (targetList[i] > upGain)
            upGain = targetList[i];
    }
    lowGain -= 5.0;
    upGain += 5.0;

    ui->obc_gain_min->setValue(lowGain);
    ui->obc_gain_max->setValue(upGain);
    free(flt_freqList);
    free(targetList);

    setStatus(true, QString::number(freq.size()) + " rows loaded");
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);

    if(ui->previewPlotContainer->isVisible()){
        updatePreviewPlot();
    }
}

void CurveFittingDialog::setStatus(bool success, QString text){
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

void CurveFittingDialog::updatePreviewPlot(){
    uint size = freq.size();

    if(size < 1){
        return;
    }

    qDebug() << "Redrawing";

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
                                   nullptr);


    double lowGain = targetList[0];
    double upGain = targetList[0];
    for (uint i = 1; i < size; i++)
    {
        if (targetList[i] < lowGain)
            lowGain = targetList[i];
        if (targetList[i] > upGain)
            upGain = targetList[i];
    }
    lowGain -= 5.0;
    upGain += 5.0;

    ui->previewPlot->yAxis->setRange(lowGain, upGain);
    ui->previewPlot->clearGraphs();

    auto pGraph0 = ui->previewPlot->addGraph();
    pGraph0->setAdaptiveSampling(true);

    for(uint i = 0; i < size; i++){
        pGraph0->addData(flt_freqList[i], (double)targetList[i]);
        qDebug() << i << ">>" << flt_freqList[i] << targetList[i];
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
                                DoubleRange(ui->obc_freq_min->value(), ui->obc_freq_max->value()),
                                DoubleRange(ui->obc_q_min->value(), ui->obc_q_max->value()),
                                DoubleRange(ui->obc_gain_min->value(), ui->obc_gain_max->value()),
                                ui->fgrid_force_convert->isChecked(),
                                ui->iterations->value(),
                                ui->iterations_simplex->value(),
                                ui->fgrid_avgbw->value(),
                                ui->rnd_pop_k->value(), ui->rnd_pop_n->value(),
                                ui->algo_fmin_dimension_adaptive->isChecked(), ui->algo_de_probibound->value(),
                                ui->algo_flower_pcond->value(), ui->algo_flower_weightstep->value(),
                                ui->algo_chio_maxsolsurvive->value(), ui->algo_chio_c0->value(), ui->algo_chio_spreadingrate->value());

    auto worker = new CurveFittingWorkerDialog(options, this);

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
    case CurveFittingOptions::AT_FMINSEARCHBND:
        ui->iterations->setValue(20000);
        break;
    case CurveFittingOptions::AT_DIFF_EVOLUTION:
        ui->iterations->setValue(20000);
        break;
    case CurveFittingOptions::AT_FLOWERPOLLINATION:
        ui->iterations->setValue(4000);
        break;
    case CurveFittingOptions::AT_CHIO:
        ui->iterations->setValue(4000);
        break;
    case CurveFittingOptions::AT_HYBRID_FLOWER_FMIN:
        ui->iterations->setValue(4000);
        ui->iterations_simplex->setValue(10000);
        break;
    case CurveFittingOptions::AT_HYBRID_DE_FMIN:
        ui->iterations->setValue(20000);
        ui->iterations_simplex->setValue(10000);
        break;
    case CurveFittingOptions::AT_HYBRID_CHIO_FMIN:
        ui->iterations->setValue(4000);
        ui->iterations_simplex->setValue(10000);
        break;
    }


    // I really need to replace this enum with a more advanced class...
    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_FMINSEARCHBND:
    case CurveFittingOptions::AT_HYBRID_FLOWER_FMIN:
    case CurveFittingOptions::AT_HYBRID_DE_FMIN:
    case CurveFittingOptions::AT_HYBRID_CHIO_FMIN:
        algo_fmin->setVisible(true);
        break;
    default:
        algo_fmin->setVisible(false);
        break;
    }

    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_HYBRID_FLOWER_FMIN:
    case CurveFittingOptions::AT_FLOWERPOLLINATION:
        algo_flower->setVisible(true);
        break;
    default:
        algo_flower->setVisible(false);
        break;
    }

    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_HYBRID_DE_FMIN:
    case CurveFittingOptions::AT_DIFF_EVOLUTION:
        algo_de->setVisible(true);
        break;
    default:
        algo_de->setVisible(false);
        break;
    }

    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_HYBRID_CHIO_FMIN:
    case CurveFittingOptions::AT_CHIO:
        algo_chio->setVisible(true);
        break;
    default:
        algo_chio->setVisible(false);
        break;
    }

    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_FMINSEARCHBND:
        advanced_rng->setVisible(false);
        break;
    default:
        advanced_rng->setVisible(true);
        break;
    }

    switch((CurveFittingOptions::AlgorithmType) index){
    case CurveFittingOptions::AT_HYBRID_FLOWER_FMIN:
    case CurveFittingOptions::AT_HYBRID_DE_FMIN:
    case CurveFittingOptions::AT_HYBRID_CHIO_FMIN: {
        QString shortcut;
        if(index == CurveFittingOptions::AT_HYBRID_DE_FMIN)
            shortcut = "de";
        else if(index == CurveFittingOptions::AT_HYBRID_FLOWER_FMIN)
            shortcut = "fpa";
        else if(index == CurveFittingOptions::AT_HYBRID_CHIO_FMIN)
            shortcut = "chio";

        QString html = "Iterations<span style='font-size:11pt; vertical-align:sub;'> " + shortcut + "</span>";

#ifdef __APPLE__
        html = OSXHtmlSizingPatch::patchTextSize(html);
#endif

        ui->iteration_label_a->setText(html);
#ifdef __APPLE__
        ui->iteration_label_b->setText(OSXHtmlSizingPatch::patchTextSize("Iterations<span style='font-size:11pt; vertical-align:sub;'> simplex</span>"));
#endif
        ui->iteration_label_b->setVisible(true);
        ui->iteration_content_b->setVisible(true);
        break;
    }
    default:
        ui->iteration_label_a->setText("Iterations");
        ui->iteration_label_b->setVisible(false);
        ui->iteration_content_b->setVisible(false);
        break;
    }

}

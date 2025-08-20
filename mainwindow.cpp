#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mygraphicsview.h"

#include <QFileSystemModel>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QGraphicsScene>
#include <QPixmap>
#include <QHBoxLayout>
#include <QProcess>
#include <QImageReader>
#include <QGraphicsPixmapItem>
#include <QDebug>

MainWindow::MainWindow(const QString &bin_folder_path, const QString &run_script_path, const QString &emulator_cli_path, QWidget *parent)
    : QMainWindow(parent)
    , m_runScript(run_script_path)
    , m_emulatorCli(emulator_cli_path)
    , ui(new Ui::MainWindow)
    , m_model(new QFileSystemModel(this))
    , m_scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    m_proc = new QProcess(this);

    // (optional) merge channels so you read both together
    // m_proc->setProcessChannelMode(QProcess::MergedChannels);

    // show logs somewhere if you have a QTextEdit/QPlainTextEdit named logEdit
    connect(m_proc, &QProcess::readyReadStandardOutput, this, [this]{
        const auto out = m_proc->readAllStandardOutput();
        // if you don't have a log widget, at least qDebug it:
        qDebug().noquote() << QString::fromLocal8Bit(out);
    });

    connect(m_proc, &QProcess::started, this, [this]{
        ui->runBtn->setEnabled(false);
        ui->stopBtn->setEnabled(true);
    });

    connect(m_proc, qOverload<int,QProcess::ExitStatus>(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus st){
                qDebug() << "Process finished. code =" << code << "exitStatus =" << st;
                ui->runBtn->setEnabled(true);
                ui->stopBtn->setEnabled(false);
            });

    connect(m_proc, &QProcess::errorOccurred, this, [this](QProcess::ProcessError e){
        qWarning() << "Process error:" << e << m_proc->errorString();
        ui->runBtn->setEnabled(true);
        ui->stopBtn->setEnabled(false);
    });

    m_model->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    m_model->setRootPath(bin_folder_path);
    m_model->setNameFilters( QStringList() << "*.bin" );
    m_model->setNameFilterDisables(false);

    ui->lineEdit->setText( bin_folder_path );
    ui->listView->setModel(m_model);
    ui->listView->setRootIndex(m_model->index(bin_folder_path));

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, [=]() {
        QString path = ui->lineEdit->text();
        if (!path.isEmpty()) {
            m_model->setRootPath(path);
            ui->listView->setRootIndex(m_model->index(path));
        }
    });

    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    QVBoxLayout *mainLayout = new QVBoxLayout(ui->graphicsView);
    mainLayout->addStretch();

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    QPushButton *minusBtn = new QPushButton("-");
    minusBtn->setFixedSize(40, 40);
    btnLayout->addWidget(minusBtn);

    QPushButton *plusBtn = new QPushButton("+");
    plusBtn->setFixedSize(40, 40);
    btnLayout->addWidget(plusBtn);

    mainLayout->addLayout(btnLayout);
    ui->graphicsView->setLayout(mainLayout);

    plusBtn->setStyleSheet(
        "QPushButton {""background-color: rgba(255, 255, 255, 100);""border: 1px solid gray;""border-radius: 5px;""}"
    );

    minusBtn->setStyleSheet(
        "QPushButton {""background-color: rgba(255, 255, 255, 100);" "border: 1px solid gray;""border-radius: 5px;""}"
    );

    connect(minusBtn, &QPushButton::clicked, this, [=]() {
        if (currentZoom > minZoom)
        {
            ui->graphicsView->scale(1.0 / 1.2, 1.0 / 1.2);
            currentZoom /= 1.2;
        }

    });

    connect(plusBtn, &QPushButton::clicked, this, [=]() {
        if (currentZoom < maxZoom)
        {
            ui->graphicsView->scale(1.2, 1.2);
            currentZoom *= 1.2;
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_selectFolderBtn_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "Select Folder", QDir::homePath());
    if (!folderPath.isEmpty())
    {
        QFileInfo info=QFileInfo(folderPath);
        ui->lineEdit->setText(info.fileName());
        m_model->setRootPath(folderPath);

        ui->listView->setRootIndex(m_model->index(folderPath));
        m_selectedFolderPath = folderPath;
    }
}

void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    m_scene->clear();
    ui->BinLine->clear();
    ui->runBtn->setEnabled(false);

    m_selectedFolderPath = m_model->filePath(index);
    QString filePath = m_model->filePath(index);
    QString jpgFile = filePath.replace( ".bin", ".jpg");

    QImageReader reader(jpgFile);
    reader.setAutoTransform(true);
    const QImage img = reader.read();
    if (img.isNull())
        return;

    const QPixmap px = QPixmap::fromImage(img);

    auto *item = m_scene->addPixmap(px);
    item->setTransformationMode(Qt::SmoothTransformation);

    m_scene->setSceneRect(px.rect());

    ui->graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    ui->graphicsView->centerOn(item);

    ui->BinLine->setText(m_model->fileName(index));
    ui->runBtn->setEnabled(true);
}

void MainWindow::on_runBtn_clicked()
{
    if (m_selectedFolderPath.isEmpty())
        return;

    // build args for your script
    QStringList args { m_emulatorCli, "--replay_file", m_selectedFolderPath };

    // (optional) set working dir to the file’s folder
    m_proc->setWorkingDirectory(QFileInfo(m_selectedFolderPath).absolutePath());

    // If your script has execute bit + shebang, this is enough:
    m_proc->start(m_runScript, args);

    // If the script is not executable or lacks a shebang, use bash:
    // m_proc->setProgram("/bin/bash");
    // m_proc->setArguments(QStringList() << m_runScript << args);
    // m_proc->start();
}

void MainWindow::on_stopBtn_clicked()
{
    if (!m_proc || m_proc->state() == QProcess::NotRunning)
        return;

    m_proc->terminate();                 // ask nicely
    if (!m_proc->waitForFinished(3000))  // give it ~3s to exit
        m_proc->kill();                  // then force kill
}


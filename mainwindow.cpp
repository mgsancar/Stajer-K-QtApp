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

MainWindow::MainWindow(const bin_folder& bin_folder, const run_script& run_script, const emulator_path& emulator, const emulator_arg& emulator_arg, QWidget *parent)
    : QMainWindow(parent)
    , m_runScript(run_script.get())
    , m_emulatorCli(emulator.get())
    , m_emulatorArg(emulator_arg.get())
    , ui(new Ui::MainWindow)
    , m_model(new QFileSystemModel(this))
    , m_scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    m_proc = new QProcess(this);
    m_proc->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_proc, &QProcess::readyReadStandardOutput, this, [this]{
        const QString chunk = QString::fromLocal8Bit(m_proc->readAllStandardOutput());
        m_scriptStdoutBuf += chunk;
        qDebug().noquote() << chunk; // anlık log görmek istersen
    });

    connect(m_proc, &QProcess::readyReadStandardError, this, [this]{
        qWarning().noquote() << QString::fromLocal8Bit(m_proc->readAllStandardError());
    });

    connect(m_proc, &QProcess::started, this, [this]{
        ui->lineEdit->setEnabled(false);
        ui->listView->setEnabled(false);
        ui->selectFolderBtn->setEnabled(false);
        ui->runBtn->setEnabled(false);
        ui->stopBtn->setEnabled(true);
        m_scriptStdoutBuf.clear();
    });

    connect(m_proc, qOverload<int,QProcess::ExitStatus>(&QProcess::finished), this, [this](int code, QProcess::ExitStatus st){
        qDebug() << "run_in_screen.sh finished. code =" << code << "exitStatus =" << st;

        // 1) Eğer sen on_runBtn_clicked içinde session adı verdiysen, onu kullan
        // 2) Aksi halde script çıktısından çek
        if (m_lastSessionName.isEmpty())
        {
            static QRegularExpression re(R"(Started screen session:\s*([^\s]+))");
            auto m = re.match(m_scriptStdoutBuf);
            if (m.hasMatch())
                m_lastSessionName = m.captured(1).trimmed();
        }

        if (m_lastSessionName.isEmpty())
            qWarning() << "Session adı alınamadı. Çıktı:" << m_scriptStdoutBuf;
        else
            printScreenInfo(m_lastSessionName);

        ui->lineEdit->setEnabled(true);
        ui->listView->setEnabled(true);
        ui->selectFolderBtn->setEnabled(true);
        ui->runBtn->setEnabled(true);
        ui->stopBtn->setEnabled(false);
        m_lastSessionName.clear();
    });

    connect(m_proc, &QProcess::errorOccurred, this, [this](QProcess::ProcessError e){
        qWarning() << "Process error:" << e << m_proc->errorString();

        ui->lineEdit->setEnabled(true);
        ui->listView->setEnabled(true);
        ui->selectFolderBtn->setEnabled(true);
        ui->runBtn->setEnabled(true);
        ui->stopBtn->setEnabled(false);
    });

    m_model->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    m_model->setRootPath(bin_folder.get());
    m_model->setNameFilters( QStringList() << "*.bin" );
    m_model->setNameFilterDisables(false);

    ui->lineEdit->setText( bin_folder.get() );
    ui->listView->setModel(m_model);
    ui->listView->setRootIndex(m_model->index(bin_folder.get()));

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, [=]() {
        QString path = ui->lineEdit->text();
        if (!path.isEmpty())
        {
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
    ui->binLine->clear();
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

    ui->binLine->setText(m_model->fileName(index));
    ui->runBtn->setEnabled(true);
}

void MainWindow::on_runBtn_clicked()
{
    if (m_selectedFolderPath.isEmpty())
        return;

    // Kendin session üret
    QString appName = QFileInfo(m_emulatorCli).baseName();
    m_lastSessionName = appName + "_" + ui->binLine->displayText().remove(".bin");

    // Script argümanları: [--session NAME] -- <APP> <APP_ARGS...>
    QStringList args { QStringList() <<
         "--session" << m_lastSessionName
         << "--"
         << m_emulatorCli << QString{"--%1"}.arg(m_emulatorArg) << m_selectedFolderPath
    };

    qDebug() << "Starting screen session with command:" << m_runScript << args;

    // (opsiyonel) çalışma dizini dosyanın bulunduğu klasör olsun
    // m_proc->setWorkingDirectory(QFileInfo(m_selectedFolderPath).absolutePath());

    // Script shebang + executable ise:
    m_proc->start(m_runScript, args);

    // Değilse:
    // m_proc->setProgram("/bin/sh");
    // m_proc->setArguments(QStringList() << m_runScript << args);
    // m_proc->start();
}

void sendLine(const QString& name, const QString& line) {
    QProcess::execute("screen", {"-S", name, "-p", "0", "-X", "stuff", line + "\r\n"});
}

void MainWindow::on_stopBtn_clicked()
{
    if (!m_proc || m_proc->state() == QProcess::NotRunning)
        return;

    if (m_lastSessionName.isEmpty())
    {
        qWarning() << "Session adı boş, durdurma işlemi yapılamıyor.";
        return;
    }

    qDebug().noquote() << "Stopping screen session:" << m_lastSessionName;
    sendLine(m_lastSessionName, "exit()"); // send exit command to script
}

static QString runAndReadAll(const QString& program, const QStringList& args,
                             int timeoutMs = 3000, int* exitCodeOut = nullptr)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(program, args);

    if (!p.waitForStarted(timeoutMs))
    {
        if (exitCodeOut) *exitCodeOut = -1;
        return QStringLiteral("[failed to start]");
    }

    p.waitForFinished(timeoutMs);

    if (exitCodeOut)
        *exitCodeOut = p.exitCode();

    return QString::fromLocal8Bit(p.readAllStandardOutput());
}

void MainWindow::printScreenInfo(const QString& sessionName)
{
    qDebug() << "=== screen -ls ===";
    int lsCode = 0;
    qDebug().noquote() << runAndReadAll("screen", {"-ls"}, 4000, &lsCode).trimmed();
    qDebug() << "screen -ls exitCode:" << lsCode;

    qDebug() << "=== screen info for" << sessionName << "===";
    int infoCode = 0;
    auto infoOut = runAndReadAll("screen", {"-S", sessionName, "-Q", "info"}, 3000, &infoCode).trimmed();
    qDebug().noquote() << infoOut;
    qDebug() << "info exitCode:" << infoCode;

    // Pencere listesi (desteklenen sürümlerde)
    int winCode = 0;
    auto winOut = runAndReadAll("screen", {"-S", sessionName, "-Q", "windows"}, 3000, &winCode).trimmed();
    if (!winOut.isEmpty())
        qDebug().noquote() << "windows:" << winOut, qDebug() << "windows exitCode:" << winCode;

    // Proses id (varsa)
    int pidCode = 0;
    auto pidOut = runAndReadAll("screen", {"-S", sessionName, "-Q", "pid"}, 3000, &pidCode).trimmed();
    if (!pidOut.isEmpty())
        qDebug().noquote() << "pid:" << pidOut, qDebug() << "pid exitCode:" << pidCode;
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mygraphicsview.h"

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

MainWindow::MainWindow(QString bin_folder_path, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new QFileSystemModel(this))
    , m_scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    m_model->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    m_model->setRootPath(bin_folder_path);
    m_model->setNameFilters(QStringList() << "*.bin");
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

void MainWindow::on_btnSelectFolder_clicked()
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

    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->BinLine->setText(m_model->fileName(index));
}

void MainWindow::on_checkBinButton_clicked()
{
    QString jpgName = ui->BinLine->text();
    QString binFile = jpgName.replace( ".jpg", ".bin");
    QDir dir(m_selectedFolderPath);

    if (dir.exists(binFile))
    {
        ui->label->setText( binFile + " found.");

        QStringList arguments;
        arguments << "--name" << "fusion";
        QString program ="yeniCli.exe";
        QProcess myProcess{};
        myProcess.start(program, arguments);
        myProcess.waitForFinished();
    }
    else
    {
        ui->label->setText(binFile + " not found.");
    }
}



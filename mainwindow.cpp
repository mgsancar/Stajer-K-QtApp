#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QGraphicsScene>
#include <QPixmap>
#include <QHBoxLayout>
#include <QProcess>
#include <QDebug>
#include <MyGraphicsView.h>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , model(new QFileSystemModel(this))
    ,scene(new QGraphicsScene(this))
{

    ui->setupUi(this);
    ui->graphicsView->setScene(scene);
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
        if (currentZoom > minZoom) {
                ui->graphicsView->scale(1.0 / 1.2, 1.0 / 1.2);
                currentZoom /= 1.2;
            }

    });
    connect(plusBtn, &QPushButton::clicked, this, [=]() {
        if (currentZoom < maxZoom) {
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

    model->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    ui->listView->setModel(model);

    QString folderPath = QFileDialog::getExistingDirectory(this, "Select Folder", QDir::homePath());
    if (!folderPath.isEmpty()) {
        QFileInfo info=QFileInfo(folderPath);
        ui->lineEdit->setText(info.fileName());
        model->setRootPath(folderPath);

        QStringList filters;
        filters <<"*.jpg"<<"*.jpeg";

        model->setNameFilters(filters);
        model->setNameFilterDisables(false);

        ui->listView->setRootIndex(model->index(folderPath));
        selectedFolderPath = folderPath;
    }
}

void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    QString filePath = model->filePath(index);
    QPixmap pixmap(filePath);

    scene->clear();
    scene->addPixmap(pixmap.scaled(ui->graphicsView->size(),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation));
    ui->graphicsView->fitInView(scene->itemsBoundingRect());

     QString fileName= model->fileName(index);
    ui->BinLine->setText(fileName);
}


void MainWindow::on_checkBinButton_clicked()
{
    QString jpgName=ui->BinLine->text();
    QString binFile =  jpgName.replace( ".jpg", ".bin");
    QDir dir(selectedFolderPath);

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



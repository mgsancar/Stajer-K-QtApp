#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QGraphicsView>
#include <QWheelEvent>
#include <QMainWindow>
#include <QFileSystemModel>
#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QString bin_folder_path, QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_listView_clicked(const QModelIndex &index   );
    void on_checkBinButton_clicked();
    void on_btnSelectFolder_clicked();
private:
    Ui::MainWindow *ui;
    QFileSystemModel *m_model;
    QGraphicsScene *m_scene;
    QString m_selectedFolderPath;
    QString m_binFile;
private:
    double currentZoom = 1.0;
    const double minZoom = 0.5;
    const double maxZoom = 3.0;
};
#endif // MAINWINDOW_H


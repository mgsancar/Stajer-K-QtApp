#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QFileSystemModel;
class QGraphicsScene;
class QProcess;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(const QString& bin_folder_path,
               const QString& run_script,
               const QString& emulator_cli,
               QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_listView_clicked(const QModelIndex &index   );
    void on_runBtn_clicked();
    void on_selectFolderBtn_clicked();

    void on_stopBtn_clicked();

private:
    Ui::MainWindow *ui;
    QProcess* m_proc = nullptr;
    QFileSystemModel *m_model;
    QGraphicsScene *m_scene;
    QString m_selectedFolderPath;
    QString m_binFile;
    const QString m_runScript;
    const QString m_emulatorCli;
    double currentZoom = 1.0;
    const double minZoom = 0.5;
    const double maxZoom = 3.0;
};
#endif // MAINWINDOW_H


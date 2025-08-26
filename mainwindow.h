#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "util/named_type.h"

class QFileSystemModel;
class QGraphicsScene;
class QProcess;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using bin_folder = NamedType<QString, struct BinFolderTag>;
using run_script = NamedType<QString, struct RunScriptTag>;
using emulator_path = NamedType<QString, struct EmulatorPathTag>;
using emulator_arg = NamedType<QString, struct EmulatorArgTag>;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(const bin_folder&,
               const run_script&,
               const emulator_path&,
               const emulator_arg&,
               QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_listView_clicked(const QModelIndex &index   );
    void on_runBtn_clicked();
    void on_selectFolderBtn_clicked();

    void on_stopBtn_clicked();

private:
    Ui::MainWindow *ui;
    const QString m_runScript;
    const QString m_emulatorCli;
    const QString m_emulatorArg;
    QProcess* m_proc = nullptr;
    QString m_scriptStdoutBuf;
    QString m_lastSessionName;
    QFileSystemModel *m_model;
    QGraphicsScene *m_scene;
    QString m_selectedFolderPath;
    QString m_binFile;

    double currentZoom = 1.0;
    const double minZoom = 0.5;
    const double maxZoom = 3.0;

    void printScreenInfo(const QString& sessionName);
};
#endif // MAINWINDOW_H


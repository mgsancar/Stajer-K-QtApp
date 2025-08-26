#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QLocale>
#include <QTranslator>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
    QApplication app(argc, argv);

    std::setlocale( LC_ALL , "en_US.UTF-8" );
    std::setlocale( LC_NUMERIC , "en_US.UTF-8" );

    QCommandLineParser cli;

    QCommandLineOption bin_folder_opt { { "bin_folder" , "bf" }, "Specify sentetich binary folder where bin files are stored.", "folder", QDir::homePath() };
    QCommandLineOption run_script_opt { { "run_script" , "rs" }, "Script file to run wetend emulator with given parameters.", "path", "script/run_in_screen.sh" };
    QCommandLineOption emulator_path_opt { { "emulator_path" , "ep" }, "Wetend Emulator path.", "path", "WetendEmulatorCLI" };
    QCommandLineOption emulator_arg_opt { { "emulator_arg" , "ea" }, "Wetend Emulator Argument.", "argument", "replay_file" };

    cli.addOptions( { bin_folder_opt,
                      run_script_opt,
                      emulator_path_opt,
                      emulator_arg_opt
                    } );
    cli.addHelpOption();
    cli.addVersionOption();

    cli.process( app );

    auto bin_folder_path = cli.value( bin_folder_opt );

    if ( !QFileInfo::exists( bin_folder_path ) )
    {
        qDebug() << "Error when processing"
                     << bin_folder_opt.names()
                     << bin_folder_path
                     << "doesn't exist.";

        return 1;
    }

    if ( !QFileInfo { bin_folder_path }.isDir() )
    {
        qDebug() << "Error when processing"
                     << bin_folder_opt.names()
                     << bin_folder_path
                     << "is not a directory";

        return 1;
    }

    auto run_script_path = cli.value( run_script_opt );

    if ( !run_script_path.isEmpty() && !QFileInfo::exists( run_script_path ) )
    {
        qDebug() << "Error when processing"
                     << run_script_opt.names()
                     << run_script_path
                     << "doesn't exist.";

        return 1;
    }

    auto emulator = cli.value( emulator_path_opt );

    if ( !QFileInfo::exists( emulator ) )
    {
        qDebug() << "Error when processing"
                     << emulator_path_opt.names()
                     << emulator
                     << "doesn't exist.";

        // return 1;
    }

    auto emulator_arg_name = cli.value( emulator_arg_opt );

    QString seperator( 100 , '=' );

    qDebug().noquote() << seperator;
    qDebug() << QApplication::applicationName() << "is starting with these settings :";
    qDebug() << "version                :" << QApplication::applicationVersion();
    qDebug() << "bin folder             :" << bin_folder_path;
    qDebug() << "run script             :" << run_script_path;
    qDebug() << "emulator path          :" << emulator;
    qDebug() << "emulator argument      :" << emulator_arg_name;

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "LayOut_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    MainWindow w( bin_folder{bin_folder_path}, run_script{run_script_path}, emulator_path{emulator}, emulator_arg{emulator_arg_name} );
    w.show();
    return app.exec();
}

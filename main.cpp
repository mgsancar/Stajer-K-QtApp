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
    QCommandLineOption run_script_opt { { "run_script" , "rs" }, "Script file to run wetend emulator with given parameters.", "path", "run_emulator.sh" };
    QCommandLineOption emulator_cli_opt { { "emulator_cli" , "ec" }, "Wetend Emulator path.", "path", "WetendEmulatorCLI" };

    cli.addOptions( { bin_folder_opt,
                      run_script_opt,
                      emulator_cli_opt
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

    auto emulator_cli_path = cli.value( emulator_cli_opt );

    if ( !QFileInfo::exists( emulator_cli_path ) )
    {
        qDebug() << "Error when processing"
                     << emulator_cli_opt.names()
                     << emulator_cli_path
                     << "doesn't exist.";

        return 1;
    }

    QString seperator( 100 , '=' );

    qDebug().noquote() << seperator;
    qDebug() << QApplication::applicationName() << "is starting with these settings :";
    qDebug() << "version                :" << QApplication::applicationVersion();
    qDebug() << "bin folder             :" << bin_folder_path;
    qDebug() << "run script             :" << run_script_path;
    qDebug() << "emulator cli           :" << emulator_cli_path;

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "LayOut_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    MainWindow w( bin_folder_path, run_script_path, emulator_cli_path );
    w.show();
    return app.exec();
}

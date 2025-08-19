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

    QCommandLineOption
        bin_folder_opt            {
            { "b" , "bin_folder" } ,
            "Specify sentetich binary folder where bin files are stored." ,
            "folder" ,
            QDir::homePath() + "/projects/sentetich/bin"
        };
    cli.addOptions( {
        bin_folder_opt
    });
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

    QString seperator( 100 , '=' );

    qDebug().noquote() << seperator;
    qDebug() << QApplication::applicationName() << "is starting with these settings :";
    qDebug() << "version                :" << QApplication::applicationVersion();
    qDebug() << "bin folder             :" << bin_folder_path;

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "LayOut_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    MainWindow w(QString ::fromStdString( bin_folder_path.toStdString() ));
    w.show();
    return app.exec();
}

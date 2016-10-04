#include "mainwindow.h"
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>
#include <QMessageBox>
#include <QApplication>
#include <QThread>
#include <QUrl>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    l_dir    = new QLabel();
    te_left  = new QTextEdit();
    te_right = new QTextEdit();
    pb_scan  = new QPushButton("Scan !");

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(te_left);
    hbox->addWidget(te_right);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(l_dir);
    vbox->addLayout(hbox);
    vbox->addWidget(pb_scan);

    setLayout(vbox);
    showMaximized();

    QObject::connect(pb_scan, SIGNAL(clicked()), this, SLOT(slot_scan()));
}

MainWindow::~MainWindow()
{

}

void MainWindow::slot_scan()
{
    int nb_errors = scan("Documents");
    if ( nb_errors == 0 )
    {
        l_dir->setText("Terminé !");
        te_left->clear();
        te_right->clear();
    }
}

// Return0 if OK
// Otherwise return nb_errors
int MainWindow::scan(const QString &dir)
{
    if ( ! QFileInfo("E:\\" + dir).isDir() || ! QFileInfo("E:\\Google Drive\\Cloud\\" + dir).isDir() )
        return 0;

    QDir dir_left  = QDir("E:\\" + dir);

    QDir dir_right = QDir("E:\\Google Drive\\Cloud\\" + dir);
    l_dir->setText(dir);
    te_left->clear();
    te_right->clear();

    QStringList tmp_files_left  = dir_left.entryList();
    QStringList tmp_files_right = dir_right.entryList();
    QStringList files_left;
    QStringList files_right;

    // PRE TRAITEMENT

    for ( int i=0 ; i<tmp_files_left.size() ; ++i )
    {
        QString str = tmp_files_left[i];
        if ( str == "." || str == ".." )
            continue;
        files_left.append(str);
    }

    for ( int i=0 ; i<tmp_files_right.size() ; ++i )
    {
        QString str = tmp_files_right[i];
        if ( str == "." || str == ".." )
            continue;

        if ( QFileInfo("E:\\Google Drive\\Cloud\\" + dir + "\\" + str).isFile() )
        {
            int last_minus = str.lastIndexOf("-");
            if ( last_minus > 0 )
            {
                QString tmp_ext = str.left(str.lastIndexOf("."));
                tmp_ext[last_minus] = '.';
                files_right.append(tmp_ext);
            }
            else
                files_right.append(str + "**** error");
        }
        else
            files_right.append(str);
    }

    // CHECK LEFT

    int nb_errors = 0;
    int count_dir = 0;

    for ( int i=0 ; i<files_left.size() ; ++i )
    {
        QString str = files_left[i];
        if ( files_right.contains(str) )
        {
            te_left->append("<font color=green>" + str + "</font>");
            te_right->append("<font color=green>" + str + "</font>");
            ++count_dir;
        }
        else
        {
            te_left->append("<font color=red>" + str + "</font>");
            ++nb_errors;
        }
    }

    // CHECK RIGHT

    for ( int i=0 ; i<files_right.size() ; ++i )
    {
        QString str = files_right[i];
        if (!  files_left.contains(str) )
        {
            te_right->append("<font color=red>" + str + "</font>");
            ++nb_errors;
        }
    }

    // Scan subdirs if no errors
    if ( nb_errors == 0 )
    {
        for ( int i=0 ; nb_errors == 0 && i<files_left.size() ; ++i )
        {
            QString str = files_left[i];
            qApp->processEvents();
            nb_errors = scan(dir + "\\" + str);
        }
    }
    // Launch explorer if errors are found
    else
    {
        QString final = dir;
        final.replace("\\", "/");
        QDesktopServices::openUrl(QUrl("file:///E:/"+final, QUrl::TolerantMode));
        QDesktopServices::openUrl(QUrl("file:///E:/Google Drive/Cloud/"+final, QUrl::TolerantMode));
    }

    qApp->processEvents();

    return nb_errors;
}

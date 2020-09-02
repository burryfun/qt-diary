#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    curFile("Diary.txt"),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createActions();

    QTabWidget* tabWidget = new QTabWidget;
    YearTab = new UserTab("Year Goals");
    MonthTab = new UserTab("Month Goals");
    tabWidget->addTab(YearTab, "Year Goals");
    tabWidget->addTab(MonthTab, "Month Goals");
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    ui->centralWidget->setLayout(mainLayout);

    init();
}

void MainWindow::init()
{
    if (file.exists(curFile))
    {
        loadFile(curFile);
    }
}

bool MainWindow::saveFile(const QString &fileName)
{
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QSaveFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        YearTab->writeToFile(file);
        MonthTab->writeToFile(file);

        if (!file.commit()) {
            errorMessage = tr("Cannot write file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString());
        }
    } else {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                       .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty()) {
        QMessageBox::warning(this, tr("Application"), errorMessage);
        return false;
    }

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved as ") + curFile, 2000);
    return true;
}

void MainWindow::loadFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);

    statusBar()->showMessage(tr("File loaded"), 2000);
    YearTab->readFromFile(fileName);
    MonthTab->readFromFile(fileName);
}

void MainWindow::readSettings()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::createActions()
{
    // FILE MENU
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* newAct = new QAction(tr("&New File"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);

    QAction* openAct = new QAction(tr("&Open..."));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);

    QAction* saveAct = new QAction(tr("&Save"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAct);

    QAction* saveAsAct = new QAction(tr("&Save As..."));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(saveAsAct);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
    {
        event->accept();
    }
    else
    {
       event->ignore();
    }
}

void MainWindow::newFile()
{
    if (maybeSave())
    {
        YearTab->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::open()
{
    if (maybeSave())
    {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.contains(".txt"))
        {
            QMessageBox::warning(this, tr("Application"),
                                              tr("File type must be .txt.\n"
                                                 "Please, choose txt file!"),
                                                    QMessageBox::Ok);
        }
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::save()
{
    if (curFile.isEmpty())
    {
        saveAs();
    }
    return saveFile(curFile);
}

bool MainWindow::saveAs()
{
    QFileDialog dialog(this);
        dialog.setWindowModality(Qt::WindowModal);
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        if (dialog.exec() != QDialog::Accepted)
            return false;
        return saveFile(dialog.selectedFiles().first());
}



bool MainWindow::maybeSave()
{
    const QMessageBox::StandardButton ret
           = QMessageBox::warning(this, tr("Application"),
                                  tr("The document has been modified.\n"
                                     "Do you want to save your changes?"),
                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
       switch (ret) {
       case QMessageBox::Save:
           return save();
       case QMessageBox::Cancel:
           return false;
       default:
           break;
       }
       return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "Diary.txt";
    setWindowFilePath(shownName);
}

MainWindow::~MainWindow()
{
    delete ui;
}

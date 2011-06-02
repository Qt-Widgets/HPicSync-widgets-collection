#include "hpicsync.h"
#include <QDebug>

HPicSync::HPicSync(QWidget *parent)
    : QMainWindow(parent),mOptionWidget(NULL),mDirManager(mOption),mMoreThanOneSelected(false)
{


    QHBoxLayout *optionBox = new QHBoxLayout();
    this->mOptionButton = new QPushButton(tr("Option"));
    optionBox->addStretch();
    optionBox->addWidget(this->mOptionButton);

    QHBoxLayout *viewBox= new QHBoxLayout();

    QVBoxLayout *newListBox= new QVBoxLayout();
    QHBoxLayout *refreshBox = new QHBoxLayout();
    this->mRefreshButton = new QPushButton(tr("Refresh"));
    refreshBox->addWidget(mRefreshButton);
    refreshBox->addStretch();

    this->mNewListWidget = new QListWidget();
    this->mNewListWidget->setFlow(QListView::LeftToRight);
    this->mNewListWidget->setWrapping(true);
    this->mNewListWidget->setUniformItemSizes(true);
    this->mNewListWidget->setResizeMode(QListView::Adjust);
    this->mNewListWidget->setAutoScroll(false);
    this->mNewListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    HPSListViewDelegate *delegateNew = new HPSListViewDelegate(this->mOption.getThumbSize(),&this->mMoreThanOneSelected,this);
    this->mNewListWidget->setItemDelegate(delegateNew);


    QHBoxLayout * buttonBox = new QHBoxLayout();
    this->mtMarkAllButton = new QPushButton(trUtf8("Alle auswählen"));
    this->mMarkSelectedButton = new QPushButton(trUtf8("Markierte auswählen"));
    buttonBox->addWidget(this->mtMarkAllButton);
    buttonBox->addWidget(this->mMarkSelectedButton);
    buttonBox->addStretch();

    newListBox->addLayout(refreshBox);
    newListBox->addWidget(mNewListWidget);
    newListBox->addLayout(buttonBox);

    this->mCopyButton = new QPushButton(tr("copy"));

    QVBoxLayout *oldListBox = new QVBoxLayout();


    QHBoxLayout *comboBox = new QHBoxLayout();

    this->mTreeComboBox = new HPSTreeCombobox(this);
    mDirManager.setModel( mTreeComboBox->standardModel());
    initCBOrdner(mOption.getComboBoxView(),mOption.getComboBoxCurrentDir());
    mPlusButton=new QPushButton(tr("plus"));
    comboBox->addWidget(mTreeComboBox,10);
    comboBox->addWidget(mPlusButton,1);

    this->mOldListWidget = new QListWidget();
    this->mOldListWidget->setFlow(QListView::LeftToRight);
    this->mOldListWidget->setWrapping(true);
    this->mOldListWidget->setUniformItemSizes(true);
    this->mOldListWidget->setResizeMode(QListView::Adjust);
    this->mOldListWidget->setAutoScroll(false);
    this->mOldListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    HPSListViewDelegate *delegateOld = new HPSListViewDelegate(this->mOption.getThumbSize(),&this->mMoreThanOneSelected,this);
    this->mOldListWidget->setItemDelegate(delegateOld);

    QHBoxLayout *beendenBox = new QHBoxLayout();
    this->mCloseButton = new QPushButton(tr("Beenden"));
    beendenBox->addStretch();
    beendenBox->addWidget(this->mCloseButton);

    oldListBox->addLayout(comboBox);
    oldListBox->addWidget(mOldListWidget);
    oldListBox->addLayout(beendenBox);
    viewBox->addLayout(newListBox);
    viewBox->addWidget(this->mCopyButton);
    viewBox->addLayout(oldListBox);

    QVBoxLayout *mainBox = new QVBoxLayout();
    mainBox->addLayout(optionBox);
    mainBox->addLayout(viewBox);

    QWidget *mainWidget = new QWidget();
    mainWidget->setLayout(mainBox);
    this->setCentralWidget(mainWidget);

    QStatusBar *bar = new QStatusBar();
    this->setStatusBar(bar);
    this->mConnectLabel = new QLabel(tr("nicht verbunden"));
    this->mConnectPixGruenLabel = new QLabel();
    this->mConnectPixGruenLabel->setPixmap(QPixmap(":/knopfGruen").scaled(QSize(17,17),Qt::KeepAspectRatio));
    this->mConnectPixRotLabel = new QLabel();
    this->mConnectPixRotLabel->setPixmap(QPixmap(":/knopfRot").scaled(QSize(17,17),Qt::KeepAspectRatio));
    mBar = new QProgressBar;
    // this->mPixOldLoadCountLabel = new QLabel("");
    //this->mPixOldLoadCountLabel->setVisible(false);
    mBar->setVisible(false);
    mBar->setValue(0);
    this->statusBar()->addWidget(this->mConnectPixGruenLabel);
    this->statusBar()->addWidget(this->mConnectPixRotLabel);
    this->statusBar()->addWidget(this->mConnectLabel);
    this->statusBar()->addPermanentWidget( mBar);

    this->connect(this->mCloseButton,SIGNAL(clicked()),this,SLOT(close()));
    this->connect(this->mOptionButton,SIGNAL(clicked()),this,SLOT(showOption()));
    this->connect(this->mRefreshButton,SIGNAL(clicked()),this,SLOT(test()));
    this->connect(this->mCopyButton,SIGNAL(clicked()),this,SLOT(test2()));
    connect( mTreeComboBox,SIGNAL(itemClicked(QModelIndex)),this,SLOT(comboBoxItemclicked(QModelIndex)));
    connect(mPlusButton,SIGNAL(clicked()),this,SLOT(clickedPlus()));
    connect( &mThumbManager,SIGNAL(thumsLoaded(int)),this,SLOT(refreshBar(int)));

    this->setGeometry(this->mOption.getGeometry());

    if(!mDatabaseHandler.openDatabase("picsync.db"))
        QMessageBox::critical(this, trUtf8("Fehler"), trUtf8("Verbindeung mit der Datenbank konnte nicht hergestellt werden."),QMessageBox::Ok);
    mThumbManager.setDatenBankHandler( &mDatabaseHandler);
    QDir dir( QApplication::applicationDirPath());
    if(!dir.exists(".thumbs")){
        dir.mkdir(".thumbs");
    }


}
HPicSync::~HPicSync()
{
    mOption.setGeometry(this->geometry());
    mOption.setComboBoxCurrentDir(mTreeComboBox->getCurrentDir());
    mOption.setExpandDirs( mTreeComboBox->expandeDirs());
}





void HPicSync::closeEvent(QCloseEvent *event){
    qDebug() << "closeEvent" << mThumbManager.allThreadsClose();
    if(mThumbManager.allThreadsClose()){
        event->accept();
    } else {
        mThumbManager.closeAllThreads();
        connect(&mThumbManager,SIGNAL(allThreadsDestroyed()),this,SLOT(close()));
        event->ignore();
    }

}



void HPicSync::showOption(){
    if(this->mOptionWidget== NULL){
        this->mOptionWidget = new HPSOptionWidget(&this->mOption,this);
        connect(mOptionWidget,SIGNAL(comboBoxViewSelectedChanged(int)),this,SLOT(comboBoxViewChanged(int)));
        //connect( mOptionWidget,SIGNAL(dirsRemoved(QStringList)), &mDirManager,SLOT(removeDirs(QStringList)));
        connect( mOptionWidget,SIGNAL(dirsRemoved(QStringList)),this,SLOT(ordnerRemoved(QStringList)));
        qDebug() << "optionWidget is null";
    }
    this->mOptionWidget->resetAndShow();
}
void HPicSync::test(){
    /* qDebug()<< "start socket";
    this->tcpModul = new HPSTCPModul(&this->option,this);
    this->tcpModul->startSearch();
*/
    // qDebug() << " start loader";
    //this->loadImages();
    //this->dirManager.showTree();
    //this->coOrdner->setViewToList();
    //this->dirManager.makeListView(this->coOrdner->model());

    // this->dirManager.makeTreeView(this->coOrdner->model());
    /*QFile file("muh.jpg");
    if(file.open(QIODevice::ReadOnly)){
        QElapsedTimer timer;
        QByteArray block = file.readAll();
        timer.start();
        QCryptographicHash::hash(block,QCryptographicHash::Sha1);
        qDebug() << timer.elapsed();
        timer.restart();
        QCryptographicHash::hash(block,QCryptographicHash::Md5);
        qDebug() << timer.elapsed();
    }else {
        qDebug() << "fehler"<< file.errorString();
    }
*/
    loadImages("C:/Users/hakah/me");

    /*QFile file("C:/Users/hakah/me/fastfertig.jpg");
    if(file.open(QIODevice::ReadOnly))
        qDebug() << true;
    else
        qDebug()<< false;
*/



}
void HPicSync::loadImages(const QString &folder){
    qDebug() << Q_FUNC_INFO;
    const int size = mThumbManager.creatThumbsAndView( mOption.getThumbSize(),folder, mOldListWidget);
    mBar->setVisible(true);
    mBar->setFormat("creating thumbnails...");
    mBar->setRange(0,size);

}



void HPicSync::socketError(QAbstractSocket::SocketError error){
    qDebug() << "error "<< error;
}
void HPicSync::test2() {
    // qDebug() << this->socket->state();


}



void HPicSync::comboBoxViewChanged(int index)
{
    initCBOrdner(index,mTreeComboBox->getCurrentDir());

}

void HPicSync::clickedPlus()
{
    const QString str = QDir::fromNativeSeparators(QFileDialog::getExistingDirectory(this,QDir::homePath()));
    //qDebug() << str;
    if (!str.isEmpty()&&!mDirManager.dirs().contains(str)) {
        mDirManager.addDir(str);
        mTreeComboBox->updateText();
    }

}

void HPicSync::initCBOrdner(int index,const QString &dir)
{
    //qDebug() << "initCBOrdner" << index << dir;
    if(index == HPSOption::ListView){
        mOption.setExpandDirs(mTreeComboBox->expandeDirs());
        mTreeComboBox->setViewToList();
    } else {
        mTreeComboBox->setViewToTree();
    }
    QList<QStandardItem*> expandesIems= mDirManager.makeView();
    if(!expandesIems.isEmpty())
        mTreeComboBox->setExpandedItems( expandesIems );
    mTreeComboBox->findeAndSetCurrentItem(dir);
}

void HPicSync::comboBoxItemclicked(QModelIndex index)
{
    qDebug() << Q_FUNC_INFO << index.data(Qt::UserRole);
}

void HPicSync::ordnerRemoved(QStringList dirs)
{

    if(dirs.contains( mTreeComboBox->getCurrentDir()))
        mTreeComboBox->setCurrentItem(NULL);

    mDirManager.removeDirs(dirs);

}

void HPicSync::saveImagesAndHashes()
{
    /*const int size = mThumbs.size();
    QFile file;
    for (int var = 0; var < size; ++var) {
        file.setFileName();

    }*/

}

void HPicSync::refreshBar(int value)
{
    mBar->setValue(value);
}

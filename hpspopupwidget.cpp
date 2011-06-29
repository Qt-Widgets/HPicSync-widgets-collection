#include "hpspopupwidget.h"
#include "ui_hpspopupwidget.h"

HPSPopupWidget::HPSPopupWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HPSPopupWidget),isVerticalOut(false),isHorizontalOut(false)
{
    ui->setupUi(this);
    initTreeView();
    initListView();

}

HPSPopupWidget::~HPSPopupWidget()
{
    qDebug() << "tot";
    delete ui;
}

QStandardItemModel * HPSPopupWidget::model()
{
    return &mModel;
}

void HPSPopupWidget::on_mTreeView_expanded(const QModelIndex &index)
{

    mModel.itemFromIndex(index)->setData(true,Qt::UserRole+1);
    QRect r =ui->mTreeView->visualRect(index);
    if(r.y()+r.height() > size().height()-40)
        isVerticalOut = true;
    else
        isVerticalOut = false;


}

void HPSPopupWidget::on_ScrollBar_RangeChanged(int min,int max)
{

    QScrollBar *bar = (QScrollBar*)sender();
    if( bar == ui->mTreeView->horizontalScrollBar()){
        if(isHorizontalOut)
            bar->setValue(max);
    } else {
        if(isVerticalOut)
            bar->setValue(max);
    }
}

void HPSPopupWidget::closeEvent(QCloseEvent *event)
{
    //qDebug() << Q_FUNC_INFO;
    emit exit();
    event->accept();
}

bool HPSPopupWidget::eventFilter(QObject *o, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress ){

        //qDebug() << Q_FUNC_INFO;
        QMouseEvent* m = static_cast<QMouseEvent*>(event);
        QModelIndex index = ui->mTreeView->indexAt(m->pos());

        QRect vrect = ui->mTreeView->visualRect(index);
        //qDebug() << vrect << m->pos();
        if(event->type() == QEvent::MouseButtonPress  &&
                vrect.contains(m->pos()))

        {
            qDebug() << index.data(Qt::UserRole).toString();

            if(index.flags().testFlag(Qt::ItemIsEnabled)){
                emit itemClicked(index.data(Qt::UserRole).toString());
                hide();
                emit exit();
            }

        }
        //qDebug() << index;
    }
    return false;
}

void HPSPopupWidget::setItem(QStandardItem *item)
{
    if(ui->mListView->isHidden()){
        ui->mTreeView->setCurrentIndex(mModel.indexFromItem(item));
    }else {
        ui->mListView->setCurrentIndex(mModel.indexFromItem(item));
    }
}


void HPSPopupWidget::on_mTreeView_activated(const QModelIndex &index)
{
    emit itemClicked(index.data(Qt::UserRole).toString());
    hide();
    emit exit();

}

void HPSPopupWidget::on_mListView_activated(const QModelIndex &index)
{
    emit itemClicked(index.data(Qt::UserRole).toString());
    hide();
    emit exit();
}

void HPSPopupWidget::initTreeView()
{

    ui->mTreeView->setTextElideMode(Qt::ElideNone);
    ui->mTreeView->header()->setResizeMode(QHeaderView::ResizeToContents);
    ui->mTreeView->viewport()->installEventFilter(this);

    connect(ui->mTreeView->verticalScrollBar(),SIGNAL(rangeChanged(int,int)),this,SLOT(on_ScrollBar_RangeChanged(int,int)));
    connect(ui->mTreeView->horizontalScrollBar(),SIGNAL(rangeChanged(int,int)),this,SLOT(on_ScrollBar_RangeChanged(int,int)));
    ui->mTreeView->hide();
}

void HPSPopupWidget::initListView()
{
    ui->mListView->hide();
}

void HPSPopupWidget::setView(int view)
{
    if( view == ListView){
        ui->mTreeView->hide();
        ui->mListView->setModel(&mModel);
        ui->mListView->show();
    }else if(view == TreeView ) {
        ui->mListView->hide();
        ui->mTreeView->setModel(&mModel);
        ui->mTreeView->show();
    }
}

void HPSPopupWidget::showAndInit()
{
    show();
    if( !ui->mListView->isHidden()){
        ui->mListView->viewport()->setFocus();
    }else{
        ui->mTreeView->viewport()->setFocus();
    }
}

void HPSPopupWidget::expanded(const QModelIndex &index)
{
    ui->mTreeView->expand(index);
}

void HPSPopupWidget::on_mTreeView_collapsed(const QModelIndex &index)
{
    mModel.itemFromIndex(index)->setData(false,Qt::UserRole+1);
}

bool HPSPopupWidget::isListView()
{
    return !ui->mListView->isHidden();
}

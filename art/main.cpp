#include <QApplication>
#include <QQmlApplicationEngine>
#include <QTimer>

#include <qqmlcontext.h>

#include "balancemodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    BalanceModel model;
    engine.rootContext()->setContextProperty("balance", &model);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

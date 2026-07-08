#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "diagnostics_model.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    DiagnosticsModel model;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("diagnostics", &model);

    const QUrl url(QStringLiteral("qrc:/torizonqt6diagnostics/QML/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}
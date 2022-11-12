#include <QApplication>
#include <QTextEdit>

#include <rpp/rpp.hpp>
#include <rppqt/rppqt.hpp>

#include <iostream>

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    QTextEdit    text_edit{};

    text_edit.setText("123123");
    text_edit.show();

    auto obs = rppqt::source::from_signal(text_edit, &QTextEdit::textChanged);
    obs.map([&text_edit](const auto&) { return text_edit.document()->toPlainText(); })
        .subscribe([](const QString& text) { std::cout << "TEXT CHANGED! : " << text.toStdString() << std::endl; },
                   []() { std::cout << "DESTROYED!" << std::endl; });

    return app.exec();
}

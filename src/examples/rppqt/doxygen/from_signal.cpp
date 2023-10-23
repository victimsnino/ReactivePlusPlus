#include <rpp/rpp.hpp>
#include <rppqt/rppqt.hpp>

#include <iostream>
#include <QApplication>
#include <QTextEdit>

/**
 * \example from_signal.cpp
 **/

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};

    //! [from_signal]
    QTextEdit* text_edit = new QTextEdit();
    rppqt::source::from_signal(*text_edit, &QTextEdit::textChanged)
        | rpp::ops::map([&](const auto&) 
          {
              return text_edit->toPlainText();
          })
        | rpp::ops::subscribe([](const QString& text) { std::cout << "text changed: " << text.toStdString() << std::endl; },
                              []() { std::cout << "text_edit destroyed!" << std::endl; });
    text_edit->setText("123");
    text_edit->setText("temp");
    delete text_edit;
    // Output:
    // text changed: 123
    // text changed: temp
    // text_edit destroyed!
    //! [from_signal]

    return 0;
}

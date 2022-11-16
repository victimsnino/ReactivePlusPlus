#include <rpp/rpp.hpp>
#include <rppqt/rppqt.hpp>


#include <iostream>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    QMainWindow  window{};
    QVBoxLayout  vbox{};
    QHBoxLayout  hbox{};

    QCheckBox lower_checkbox{"Insert all characters as upper"};
    QCheckBox remove_spaces_checkbox{"Insert text without spaces"};
    QTextEdit text_input{};
    text_input.setPlaceholderText("Text from this area would be inserted to result with effects based on flags");
    QPushButton push_text{"Push text to result"};
    QLabel      result_separator{"Result: \n(Red text is preview of text which would be appended)"};
    QTextEdit   result{};
    result.setReadOnly(true);

    hbox.addWidget(&lower_checkbox);
    hbox.addWidget(&remove_spaces_checkbox);
    vbox.addLayout(&hbox);
    vbox.addWidget(&text_input);
    vbox.addWidget(&push_text);
    vbox.addWidget(&result_separator);
    vbox.addWidget(&result);


    window.setCentralWidget(new QWidget);
    window.centralWidget()->setLayout(&vbox);

    window.show();

    auto remove_spaces_obs = rppqt::source::from_signal(remove_spaces_checkbox, &QCheckBox::stateChanged).
            start_with(static_cast<int>(remove_spaces_checkbox.isChecked()));
    auto lower_case_obs = rppqt::source::from_signal(lower_checkbox, &QCheckBox::stateChanged).
            start_with(static_cast<int>(lower_checkbox.isChecked()));

    auto text_to_append_obs = rppqt::source::from_signal(text_input, &QTextEdit::textChanged)
                              .map([&](const auto&)
                              {
                                  return text_input.toPlainText();
                              })
                              .combine_latest([](const QString& current_string, bool is_remove_spaces)
                                              {
                                                  return is_remove_spaces ? current_string.simplified().remove(' ') : current_string;
                                              },
                                              std::move(remove_spaces_obs))
                              .combine_latest([](const QString& current_string, bool is_lower)
                                              {
                                                  return is_lower ? current_string.toUpper() : current_string;
                                              },
                                              std::move(lower_case_obs))
                              .tap([](const QString& text) { std::cout << "Text to append: " << text.toStdString() << std::endl; })
                              .publish()
                              .ref_count();


    rppqt::source::from_signal(push_text, &QPushButton::pressed)
            .with_latest_from([&](const auto&, const QString& text_to_append)
                              {
                                  return text_to_append;
                              },
                              text_to_append_obs)
            .tap([&](const auto&) { text_input.setText(""); })
            .scan(QString{},
                  [](QString&& seed, const QString& end_text)
                  {
                      seed.append(end_text);
                      return std::move(seed);
                  })
            .start_with(QString{})
            .combine_latest([&](const QString& current_text, const QString& preview_text_to_append)
                            {
                                return current_text + QString("<span style=\" color:#ff0000;\">%1</span>").arg(preview_text_to_append);
                            },
                            text_to_append_obs)
            .subscribe([&](const QString& current_text)
                       {
                           result.setText(current_text);
                       },
                       [](const std::exception_ptr& err)
                       {
                           try
                           {
                               std::rethrow_exception(err);
                           }
                           catch (const std::exception& e)
                           {
                               qCritical("Unhandled exception:\n%s", e.what());
                           }
                       });

    return app.exec();
}

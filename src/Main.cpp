#include <tracktion_engine/tracktion_engine.h>

#include <memory>

#include "MainComponent.h"

class TinyDawApplication final : public juce::JUCEApplication {
 public:
  TinyDawApplication() = default;
  ~TinyDawApplication() override = default;
  const juce::String getApplicationName() override {
    return JUCE_APPLICATION_NAME_STRING;
  }
  const juce::String getApplicationVersion() override {
    return JUCE_APPLICATION_VERSION_STRING;
  }
  bool moreThanOneInstanceAllowed() override { return true; }

  //==============================================================================
  void initialise(const juce::String&) override {
    mainWindow = std::make_unique<MainWindow>(getApplicationName(),
                                              new MainComponent(), *this);
  }
  void shutdown() override { mainWindow = nullptr; }

  //==============================================================================
  void systemRequestedQuit() override { quit(); }

  void anotherInstanceStarted(const juce::String& commandLine) override {
    juce::ignoreUnused(commandLine);
  }

  class MainWindow final : public juce::DocumentWindow {
   public:
    explicit MainWindow(const juce::String& name, juce::Component* c,
                        juce::JUCEApplication& appInstance)
        : DocumentWindow(name, juce::Colours::lightgrey,
                         DocumentWindow::allButtons),
          app(appInstance) {
      setUsingNativeTitleBar(true);
      setContentNonOwned(c, true);
      setResizable(true, true);
      centreWithSize(getWidth(), getHeight());
      setVisible(true);
    }

    void closeButtonPressed() override { app.systemRequestedQuit(); }

   private:
    juce::JUCEApplication& app;
  };

 private:
  std::unique_ptr<MainWindow> mainWindow;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TinyDawApplication)
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(TinyDawApplication)

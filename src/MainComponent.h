#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "tracktion_engine/tracktion_engine.h"

// CustomToolbarItemFactory
class CustomToolbarItemFactory : public juce::ToolbarItemFactory {
 public:
  enum ToolbarItemIds { playButton = 1, stopButton };

  // ツールバーに含まれるすべてのアイテムの ID を定義
  void getAllToolbarItemIds(juce::Array<int>& ids) override {
    ids.add(playButton);
    ids.add(stopButton);
  }

  // デフォルトで表示するアイテムセットを定義
  void getDefaultItemSet(juce::Array<int>& ids) override {
    ids.add(playButton);  // 再生ボタン
    ids.add(stopButton);  // 停止ボタン
  }

  // 特定の ID に基づいてツールバーアイテムを生成
  juce::ToolbarItemComponent* createItem(int itemId) override {
    if (itemId == playButton) {
      auto playIcon = createPlayIcon();
      return new juce::ToolbarButton(playButton, "Play", std::move(playIcon),
                                     nullptr);
    } else if (itemId == stopButton) {
      auto stopIcon = createStopIcon();
      return new juce::ToolbarButton(stopButton, "Stop", std::move(stopIcon),
                                     nullptr);
    }

    return nullptr;  // 不明な ID の場合は null を返す
  }

 private:
  // 仮のアイコン（再生ボタン）を生成
  std::unique_ptr<juce::Drawable> createPlayIcon() {
    auto drawable = std::make_unique<juce::DrawablePath>();
    juce::Path path;
    path.addTriangle(0.0f, 0.0f, 100.0f, 50.0f, 0.0f, 100.0f);  // 再生の三角形
    drawable->setPath(path);
    drawable->setFill(juce::Colours::green);
    return drawable;
  }

  // 仮のアイコン（停止ボタン）を生成
  std::unique_ptr<juce::Drawable> createStopIcon() {
    auto drawable = std::make_unique<juce::DrawablePath>();
    juce::Path path;
    path.addRectangle(0.0f, 0.0f, 100.0f, 100.0f);  // 四角形
    drawable->setPath(path);
    drawable->setFill(juce::Colours::red);
    return drawable;
  }
};

// MainComponent
class MainComponent : public juce::Component, public juce::MenuBarModel {
 private:
  const int width = 800;
  const int height = 600;

 public:
  MainComponent() : engine("TinyDAW") {
    setSize(width, height);  // ウィンドウサイズの設定
    addAndMakeVisible(menuBar);
    menuBar.setModel(this);

    toolbarFactory = std::make_unique<CustomToolbarItemFactory>();
    addAndMakeVisible(toolbar);
    toolbar.addDefaultItems(*toolbarFactory);

    createEmptyProject();
  }
  ~MainComponent() override { menuBar.setModel(nullptr); }

  juce::StringArray getMenuBarNames() override {
    return {"File", "Edit", "Help"};
  }

  juce::PopupMenu getMenuForIndex(int menuIndex, const juce::String&) override {
    juce::PopupMenu menu;
    if (menuIndex == 0) {
      menu.addItem("New Project", [this] { createEmptyProject(); });
      menu.addItem("Open...", [this] { openProject(); });
      menu.addItem("Save", [this] { saveProject(); });
      menu.addSeparator();
      menu.addItem("Exit", [] {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
      });
    }
    return menu;
  }

  void menuItemSelected(int menuIndex, int) override {}

  void resized() override {
    auto bounds = getLocalBounds();
    menuBar.setBounds(bounds.removeFromTop(20));
    toolbar.setBounds(bounds.removeFromTop(40));
  }

  void paint(juce::Graphics& g) override {
    g.fillAll(juce::Colours::darkgrey);  // 背景色を設定
    g.setColour(juce::Colours::white);
    g.drawText("Hello, MiniDAW!", getLocalBounds(),
               juce::Justification::centred, true);
  }

 private:
  tracktion::Engine engine;
  std::unique_ptr<tracktion::Edit> edit;

  juce::MenuBarComponent menuBar;
  juce::Toolbar toolbar;
  std::unique_ptr<CustomToolbarItemFactory> toolbarFactory;

  void createEmptyProject() {
    auto editFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                        .getChildFile("TempEdit.edit");
    edit = tracktion::createEmptyEdit(engine, editFile);
    juce::Logger::writeToLog("Empty Project Created!");
  }

  void saveProject() {
    if (edit) {
      auto saveFile =
          juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
              .getChildFile("SavedProject.edit");
      edit->flushState();
      auto outputStream = saveFile.createOutputStream();
      edit->state.writeToStream(*outputStream);
      juce::Logger::writeToLog("Project Saved to: " +
                               saveFile.getFullPathName());
    }
  }
  void openProject() {
    auto chooser = std::make_unique<juce::FileChooser>(
        "Open Project",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.edit");
    if (chooser->browseForFileToOpen()) {
      auto file = chooser->getResult();
      edit = tracktion::loadEditFromFile(engine, file);
      juce::Logger::writeToLog("Project Opened: " + file.getFullPathName());
    }
  }
};

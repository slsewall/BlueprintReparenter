# 🧩 Blueprint Reparenter (Unreal Engine Plugin)

Easily reparent any Blueprint directly from the **Content Browser** with a single right-click.  
This editor plugin adds a **“Reparent Blueprint…”** option that opens a class picker and safely updates the Blueprint’s parent class.

This solves the whole "You can't reparent to the following parent because unreal doesn't think it matches up perfectly."

Anyway, I crashed out and wrote this. 

---

## ✨ Features
- Adds a **right-click context menu** entry to the Content Browser  
- Opens a **class picker** to select a new parent class  
- Works for any Blueprint (C++ or BP-based)  
- Automatically recompiles and saves the Blueprint after reparenting  
- Editor-only — does not affect packaged builds

---

## 🧰 Requirements
- Unreal Engine **5.6 or newer** Not tested on prior to 5.4
- Works in **Editor builds only**
- C++ project setup (plugin includes C++ module)

---

## 📦 Installation

### Option 1 – Per-project
1. Copy this folder into your project:

<YourProject>/Plugins/BlueprintReparenter/

2. Regenerate project files (`.uproject → right-click → Generate Visual Studio project files`).
3. Build your project.
4. Launch Unreal Editor and go to **Edit → Plugins → Project → Blueprint Reparenter**.
5. Enable the plugin and restart the editor.

### Option 2 – Engine-wide
1. Place the folder in your Engine plugins directory:

<Engine>/Engine/Plugins/Editor/BlueprintReparenter/

2. Rebuild your engine (or project) and enable it in any project.

---

## 🧭 Usage

1. In the **Content Browser**, locate a Blueprint asset.
2. **Right-click** it.
3. Choose **Reparent Blueprint…**
4. A **Class Picker** window appears.
5. Select the new parent class.
6. The Blueprint will:
- Update its parent class
- Recompile automatically
- Mark itself dirty for saving

---

## ⚙️ Technical Notes

- Uses Unreal’s module extension system to add context-menu entries.  
- Hooks into `FContentBrowserModule::GetAllAssetViewContextMenuExtenders()`.  
- Reparenting is done via:
```cpp
BP->ParentClass = NewParentClass;
FKismetEditorUtilities::CompileBlueprint(BP);
BP->MarkPackageDirty();
BP->PostEditChange();

    Safe for all editor Blueprints; no runtime dependencies.

    Plugin type: "Editor"

    Loading phase: "PostEngineInit"
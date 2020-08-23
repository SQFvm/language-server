import { ExtensionContext, TextEditor, Range, Position } from "vscode";
import * as vscode from "vscode";
import * as path from "path";
import * as fs from "fs";


// This allows you to select a path in any text file and if this is
// a valid path on the <Workdrive> drive then open it in VSCODE
export function activate(context: ExtensionContext): void {
    let disposable: vscode.Disposable = vscode.commands.registerCommand("sqfVmLanguageServer.openFilePath", () => {
        const editor: TextEditor | undefined = vscode.window.activeTextEditor;
        
        if (!editor) {
            return
        }
        if (editor.selection.isEmpty) {
            vscode.window.showInformationMessage("Select the path you want to open")
            return
        }
        
        // Get the selected text
        let start: Position = editor.selection.start;
        let end: Position = editor.selection.end;
        let file_path: string = editor.document.getText(new Range(start, end));
        
        // Form it to a path on workdrive
        let work_drive: string | undefined = vscode.workspace.getConfiguration().get("sqfVmLanguageServer.workdrive.path")
        if (!work_drive) {
            vscode.window.showWarningMessage("The workdrive.path needs to be set in User / Workspace settings")
            return
        }
        file_path = file_path.replace(/\"/g, "")
        if (!file_path.startsWith(work_drive)) {
            file_path = path.join(work_drive, file_path);
        };

        file_path = path.normalize(file_path)
        if (!fs.existsSync(file_path)) {
            vscode.window.showInformationMessage("File doesn't exist at " + file_path);
            return;
        }

        open_file(file_path);
    });

    context.subscriptions.push(disposable);
}

// Opens the file and moves cursor to bottom
function open_file(path: string): void {
    vscode.workspace.openTextDocument(path).then((document) => {
        vscode.window.showTextDocument(document, 1, false).then((editor) => {});
    });
}

import { ExtensionContext } from "vscode";
import * as vscode from "vscode";
import * as fs from "fs";
import * as fg from "fast-glob";
import { appdataFolder} from "../utils/engine"


// This is to open up the latest RPT file from our VBS3_Diag64 in appdata
// Registers command that lets us open up the latest RPT file
export function activate(context: ExtensionContext): void {
    let disposable: vscode.Disposable = vscode.commands.registerCommand("sqfVmLanguageServer.openRpt", () => {

        // If appdata is not present, this is probably linux in which case we won't have normal rpt files anyway
        const rpt_path: string | undefined = appdataFolder();
        if (!rpt_path) {
            vscode.window.showWarningMessage("No appdata folder to find rpt files in");
            return;
        }

        if (!fs.existsSync(rpt_path)) {
            vscode.window.showWarningMessage(".rpt Folder doesn't exist at: " + rpt_path);
            return;
        }

        // Get all rpt files, and sort them by timestamp metadata
        const latest_files = fg.sync("*.rpt", { cwd: rpt_path, caseSensitiveMatch: false, absolute: true }).map((name) => ({ name, ctime: fs.statSync(name).ctime }));
        if (latest_files.length == 0) {
            vscode.window.showWarningMessage("No .rpt files found");
            return;
        }

        let latest_file: string = latest_files.sort((a, b) => b.ctime.getTime() - a.ctime.getTime())[0].name;
        open_file(latest_file);
    });

    context.subscriptions.push(disposable);
}

// Opens the file and moves cursor to bottom
function open_file(path: string): void {
    vscode.workspace.openTextDocument(path).then((document) => {
        vscode.window.showTextDocument(document, 1, false).then((editor) => {
            vscode.commands.executeCommand("cursorBottom");
        });
    });
}

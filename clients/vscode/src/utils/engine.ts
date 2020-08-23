import * as vscode from "vscode";
import * as path from "path";

/**
 * Gets the appdata folder for the currently selected engine mode
 */
export function appdataFolder(): string | undefined {
    const appdata: string | undefined = process.env.LOCALAPPDATA;
    if (!appdata) {
        return undefined;
    };

    const gamemode: string | undefined = vscode.workspace.getConfiguration().get("sqfVmLanguageServer.mode")
    let folder_name: string | undefined = undefined;
    switch (gamemode) {
        case "Arma2":
            folder_name = "Arma 2";
            break;
        case "Arma3":
            folder_name = "Arma 3";
            break
        case "VBS3":
            folder_name = "VBS3_Diag64";
            break
        case "VBS4":
            folder_name = "VBS4_Diag64";
        default:
            break;
    }
    if (!folder_name) {
        return undefined;
    }
    return path.join(appdata, folder_name)
}

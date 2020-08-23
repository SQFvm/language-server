import { ExtensionContext, TextEditor, Range, Position, Selection } from "vscode";
import * as vscode from "vscode";
import * as path from "path";
import * as fs from "fs";


function aligner(current: string): string | undefined {
    let lines: string[] = current.split("\n")

    // Finds the equal sign furthest to the right
    let equal_index: number = -1;
    for (let line of lines) {
        let equal_id: number | undefined = line.indexOf("=");
        if (equal_id) {
            if (equal_id > equal_index) {
                equal_index = equal_id;
            };
        };
    };

    if (equal_index == -1) {
        vscode.window.showInformationMessage("No equal signs detected in selected text");
        return
    };

    lines.forEach((value, index, array) => {
        let equal_id: number | undefined = value.indexOf("=");
        if (equal_id) {
            let after: string = value.substring(equal_id + 1);
            let before: string = value.substring(0, equal_id);
            array[index] = before.padEnd(equal_index, " ") + "=" + after;
        }
    })

    return lines.join("\n")
}

function alignEqualSigns(): void {
    const editor: TextEditor | undefined = vscode.window.activeTextEditor;
    if (!editor) {
        return
    }
    if (editor.selection.isEmpty) {
        vscode.window.showInformationMessage("Please select the text you want to align")
        return
    }
    
    // Get the selected text
    const selection: Selection = editor.selection;
    let start: Position = selection.start;
    let end: Position = selection.end;
    let text: string = editor.document.getText(new Range(start, end));

    editor.edit(editBuilder => {
        let edited: string | undefined = aligner(text);
        if (edited) {
            editBuilder.replace(selection, edited);
        }
    })
}

export function activate(context: ExtensionContext): void {
    context.subscriptions.push(vscode.commands.registerCommand("sqfVmLanguageServer.alignEquals", alignEqualSigns));
}
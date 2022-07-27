import * as vscode from "vscode";

function getImportUri(
	workspace: vscode.WorkspaceFolder,
	document: vscode.TextDocument,
	position: vscode.Position
): vscode.Location | undefined {

	// See if the targeted word is "import"
	const word = document.getText(document.getWordRangeAtPosition(position));
	if (word != "import") {
		return;
	}

	// Get the string after the "import" token
	const pathPos = position.translate(0, 7);
	const pathRange = document.getWordRangeAtPosition(pathPos, /"(.*)"/);
	if (!pathRange) {
		return;
	}

	// Remove the quotes from the path
	let path = document.getText(pathRange);
	path = path.substring(1, path.length - 1);

	// Return the URI of the imported file
	return new vscode.Location(
		vscode.Uri.file(`${workspace.uri.path}/${path}.scope`),
		new vscode.Position(0, 0)
	);
}

export function activate(context: vscode.ExtensionContext) {
	const importLink = vscode.languages.registerDefinitionProvider("scope", {
		provideDefinition(document, position) {
			// Get the workspace folder of the document
			const workspace = vscode.workspace.getWorkspaceFolder(document.uri);
			if (!workspace) {
				return;
			}

			return getImportUri(workspace, document, position);
		}
	});

	context.subscriptions.push(importLink);
}
export default { lines, linesAndProgress }


function* lines(text: string) {
    let cursor = 0;
    let index = 0;
    let skipNextChar = false;

    for( const c of text ) {
        if (skipNextChar) {
            skipNextChar = false;
            continue;
        }
        if (c === '\r') {
            // Windows style.
            yield text.substr(cursor, index - cursor);
            cursor = index + 2;
            index++;
            skipNextChar = true;
        }
        else if (c === '\n'){
            // UNIX style
            yield text.substr(cursor, index - cursor);
            cursor = index + 1;
        }
        index++;
    }
    if (cursor < text.length) yield text.substr(cursor);
}


function* linesAndProgress(text: string) {
    const size = text.length
    let cursor = 0
    let index = 0
    let skipNextChar = false
    let lineNumber = 1

    for( const c of text ) {
        index++;
        if (skipNextChar) {
            skipNextChar = false
            continue;
        }
        if (c === '\r') {
            // Windows style.
            yield {
                line: text.substr(cursor, index - cursor),
                lineNumber,
                progress: index / size
            }
            lineNumber++
            cursor = index + 2
            skipNextChar = true
        }
        else if (c === '\n'){
            // UNIX style
            yield {
                line: text.substr(cursor, index - cursor),
                lineNumber,
                progress: index / size
            }
            lineNumber++
            cursor = index + 1
        }
    }
    if (cursor < text.length) yield {
        line: text.substr(cursor),
        progress: 1
    }
}

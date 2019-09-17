const RX_SEPARATOR = /[ \t\n\r,.|<>:]+/g;

/**
 * ```
 * const m = new Matcher("Pi cell");
 * m.matches('This is a cells pie!') === true
 * m.matches('I bought pins and a cellular.') === 'true'
 * m.matcher('What a nive cellular!') === 'false'
 * ```
 */
export default class Matcher {
  private items: string[];

  constructor(textToMatchAgainst: string) {
    this.items = textToMatchAgainst
      .split(RX_SEPARATOR)
      .map( (item: string) => item.trim().toLowerCase() )
      .filter( (item: string) => item.length > 0 );
  }

  matches(text: string) {
    if( this.items.length === 0 ) return true;
    const lowerCaseText = text.toLowerCase();
    for( const item of this.items ) {
      const pos = lowerCaseText.indexOf( item );
      if( pos === -1 ) return false;
    }
    return true;
  }
}

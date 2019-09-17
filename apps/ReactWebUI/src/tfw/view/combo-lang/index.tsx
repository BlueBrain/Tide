import React from "react"
import Combo from "../combo"
import Icon from "../icon"
import Flex from "../../layout/flex"
import castStringArray from "../../converter/string-array"
import languages from "../../intl.json"

interface IComboLangProps {
    highlight?: string[],
    onChange?: (lang: string) => void,
    label?: string,
    value: string,
    wide?: boolean
}

export default class ComboLang extends React.Component<IComboLangProps, {}> {
    constructor( props: IComboLangProps ) {
        super( props );
        this.handleChange = this.handleChange.bind( this );
    }

    handleChange(lang: string) {
        const handler = this.props.onChange;
        if (typeof handler !== 'function') return;
        try {
            handler(lang);
        } catch(ex) {
            console.error("Error in handleChange(lang): ", lang);
            console.error(ex);
        }
    }

    render() {
        const highlight = castStringArray(this.props.highlight);
        const comboItems = highlight.map( (lang: string) => (
            <Flex key={lang} dir="row" wrap="nowrap"
                justifyContent="space-between" alignItems="center">
                <Icon content={`flag-${lang}`} />
                <b>{languages[lang]}</b>
            </Flex>
        ));
        const otherLanguages = Object.keys(languages)
            .filter( (lang: string) => highlight.indexOf(lang) === -1 )
            .map( (lang: string) => (
                <Flex key={lang} dir="row" wrap="nowrap"
                        justifyContent="space-between" alignItems="center">
                    <div>{languages[lang]}</div>
                    <Icon content={`flag-${lang}`} />
                </Flex>
            ))
        comboItems.push(...otherLanguages);
        return <Combo
                wide={this.props.wide}
                label={this.props.label}
                value={this.props.value}
                onChange={this.handleChange}>{
                    comboItems
                }</Combo>
    }
}

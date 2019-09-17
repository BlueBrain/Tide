import "./theme.css"
import Color from "./color"

/**
 * Manage CSS styles.
 */
export default {
    register: registerTheme,
    apply: applyTheme,
    get: getTheme,
    bg0: makeGetCurrentColor("bg0"),
    bg1: makeGetCurrentColor("bg1"),
    bg2: makeGetCurrentColor("bg2"),
    bg3: makeGetCurrentColor("bg3"),
    bgP: makeGetCurrentColor("bgP"),
    bgPD: makeGetCurrentColor("bgPD"),
    bgPL: makeGetCurrentColor("bgPL"),
    bgS: makeGetCurrentColor("bgS"),
    bgSD: makeGetCurrentColor("bgSD"),
    bgSL: makeGetCurrentColor("bgSL"),
    black: makeGetCurrentColor("black"),
    white: makeGetCurrentColor("white"),
    isDark,
    setFontSize,
    normalize: completeWithDefaultValues
};

interface IStyle {
    bg0?: string;
    bg1?: string;
    bg2?: string;
    bg3?: string;
    bgP?: string;
    bgPD?: string;
    bgPL?: string;
    bgS?: string;
    bgSD?: string;
    bgSL?: string;
    fg0?: string;
    fg1?: string;
    fg2?: string;
    fg3?: string;
    fgP?: string;
    fgPD?: string;
    fgPL?: string;
    fgS?: string;
    fgSD?: string;
    fgSL?: string;
    black?: string;
    white?: string;
    [key: string]: string | undefined;
};

interface ITheme {
    bg0: string,
    bg1: string,
    bg2: string,
    bg3: string,
    bgP: string,
    bgPL: string,
    bgPD: string,
    bgS: string,
    bgSL: string,
    bgSD: string,
    black: string,
    white: string
}


//################################################################################

interface IThemes {
    css: { [name: string]: HTMLStyleElement };
    vars: { [name: string]: IStyle }
    current: string | null;
}

// Used for luminance computations. Because we need to know which text color
// can be used based on the background's luminance.
const COLOR = new Color();

const THEME_COLOR_NAMES = ["0", "1", "2", "3", "P", "PD", "PL", "S", "SD", "SL"];
const THEMES: IThemes = {
    css: {},
    vars: {},
    current: null
};

function registerTheme(themeName: string, _style: IStyle) {
    const style = completeWithDefaultValues(_style);
    THEMES.vars[themeName] = style;

    let codeCSS = codeVariables(themeName, style);
    codeCSS += codeBackground(themeName, style);
    codeCSS += codeElevation(themeName, style);
    codeCSS += codeText(themeName, style);

    let styleElement = THEMES.css[themeName];
    if (!styleElement) {
        styleElement = document.createElement("style");
        document.getElementsByTagName('head')[0].appendChild(styleElement);
        THEMES.css[themeName] = styleElement;
    }

    styleElement.textContent = codeCSS;
}

function codeText(themeName: string, style: IStyle) {
    let codeCSS = '';
    for (let depth = 1; depth <= 10; depth++) {
        for (const colorName of THEME_COLOR_NAMES) {
            const fgColorName = `fg${colorName}`;
            const bgColorName = `bg${colorName}`;
            const styleFgColorName: string = style[fgColorName] as string;
            const styleBgColorName: string = style[bgColorName] as string;
            const bgClass = ".thm-bg" + colorName;
            const fgClass = ".thm-fg" + colorName;
            for (let position = 1; position <= depth; position++) {
                const piecesFG = [];
                const piecesSVG = [];
                const piecesBG = [];
                for (let index = 1; index <= depth; index++) {
                    piecesBG.push(position === index ? bgClass : '*');
                    piecesSVG.push(position === index ? bgClass : '*');
                    piecesFG.push(position === index ? fgClass : '*');
                    if (index === depth) {
                        piecesBG.push(piecesBG.pop() + ".thm-fg");
                        piecesFG.push(piecesFG.pop() + fgClass);
                    }
                }
                codeCSS += "body.dom-theme-" + themeName + " "
                    + piecesBG.join(" > ")
                    + " { color: " + styleFgColorName + " }\n";
                codeCSS += "body.dom-theme-" + themeName + " "
                    + removeTail(piecesBG.join(" > "), ".thm-fg")
                    + " { color: " + styleFgColorName + " }\n";
                codeCSS += "body.dom-theme-" + themeName + " "
                    + piecesFG.join(" > ")
                    + " { color: " + styleBgColorName + " }\n";
                codeCSS += "body.dom-theme-" + themeName + " "
                    + piecesSVG.join(" > ")
                    + " .thm-svg-fill0"
                    + " { fill: " + styleFgColorName + " }\n";
                codeCSS += "body.dom-theme-" + themeName + " "
                    + piecesSVG.join(" > ")
                    + " .thm-svg-stroke0"
                    + " { stroke: " + styleFgColorName + " }\n";
            }
        }
    }
    return codeCSS;
}

const ALPHA_HEXA = "123456789ABCDE";
function codeVariables(themeName: string, style: IStyle) {
    let codeCSS = "body.dom-theme-" + themeName + '{\n';
    THEME_COLOR_NAMES.forEach(function(colorName) {
        const s = style[`bg${colorName}`] as string;
        codeCSS += `  --thm-bg${colorName}: ${s};\n`;
        for (const a of ALPHA_HEXA) {
            codeCSS += `  --thm-bg${colorName}-${a}: ${s}${a}${a};\n`;
        }
        COLOR.parse(s);
        const pen = COLOR.luminanceStep() ? style.black : style.white;
        codeCSS += `  --thm-fg${colorName}: ${pen};\n`;
        for (const a of ALPHA_HEXA) {
            codeCSS += `  --thm-fg${colorName}-${a}: ${pen}${a}${a};\n`;
        }
    });
    style.white = normalize(style.white);
    style.black = normalize(style.black);
    codeCSS += `  --thm-white: ${style.white};\n`;
    codeCSS += `  --thm-black: ${style.black};\n`;
    for (const letter of ALPHA_HEXA) {
        codeCSS += `  --thm-white-${letter}: ${style.white}${letter}${letter};\n`;
        codeCSS += `  --thm-black-${letter}: ${style.black}${letter}${letter};\n`;
    }
    codeCSS += "}\n";
    return codeCSS;
}


function codeBackground(themeName: string, style: IStyle) {
    var codeCSS = '';
    THEME_COLOR_NAMES.forEach(function(colorName) {
        codeCSS += "body.dom-theme-" + themeName + ".thm-bg" + colorName
            + " { background-color: " + style[`bg${colorName}`] + " }\n";
        codeCSS += "body.dom-theme-" + themeName + " .thm-fg" + colorName
            + " { color: " + style[`fg${colorName}`] + " }\n";
        codeCSS += "body.dom-theme-" + themeName + " .thm-bg" + colorName
            + " { background-color: " + style[`bg${colorName}`] + " }\n";
        codeCSS += `body.dom-theme-${themeName} .thm-bg${colorName}-text`
            + " { background-color: " + style[`fg${colorName}`] + " }\n";
        codeCSS += "body.dom-theme-" + themeName + " .thm-bg" + colorName + "-bottom"
            + " { background: linear-gradient(to top,"
            + style[`bg${colorName}`] + ",transparent) }\n";
        codeCSS += "body.dom-theme-" + themeName + " .thm-bg" + colorName + "-top"
            + " { background: linear-gradient(to bottom,"
            + style[`bg${colorName}`] + ",transparent) }\n";
        codeCSS += "body.dom-theme-" + themeName + " .thm-bg" + colorName + "-left"
            + " { background: linear-gradient(to right,"
            + style[`bg${colorName}`] + ",transparent) }\n";
        codeCSS += "body.dom-theme-" + themeName + " .thm-bg" + colorName + "-right"
            + " { background: linear-gradient(to left,"
            + style[`bg${colorName}`] + ",transparent) }\n";

        if (!isNaN(parseInt(colorName))) return;
        codeCSS += "body.dom-theme-" + themeName + " .thm-svg-fill" + colorName
            + " { fill: "
            + style[`bg${colorName}`] + " }\n";
        codeCSS += "body.dom-theme-" + themeName + " .thm-svg-stroke" + colorName
            + " { stroke: "
            + style[`bg${colorName}`] + " }\n";
    });
    return codeCSS;
}

const ELEVATIONS: { [ele: string]: string[] } = {
    "0": ["none"],
    "2": ["button", "card"],
    "4": ["bar"],
    "6": ["floating"],
    "8": ["button:active", "button-raised", "card-raised"],
    "9": ["sunmenu-0"],
    "10": ["sunmenu-1"],
    "11": ["sunmenu-2"],
    "12": ["floating:active"],
    "16": ["nav"],
    "24": ["dialog"]
};
function codeElevation(themeName: string, style: IStyle) {
    COLOR.parse(style.bg2);
    const luminance = COLOR.luminance();
    var elevationColor = luminance < .6
        ? addAlpha(style.white, 4)
        : addAlpha(style.black, 6);
    var codeCSS = '';
    const elevationKeys = Object.keys(ELEVATIONS);
    elevationKeys.forEach(function(elevationKey) {
        const elevation = parseInt(elevationKey, 10);
        ELEVATIONS[elevationKey].forEach(name => {
            codeCSS += `body.dom-theme-${themeName} .thm-ele-${name} {
  box-shadow: 0 ${elevation}px ${2 * elevation}px ${elevationColor}
}\n`;
        })
    });
    return codeCSS;
}

function applyTheme(name: string, target: HTMLElement = document.body) {
    if (!THEMES.css[name]) {
        console.error("This theme has not been registered: ", name);
        return;
    }
    var body = document.body;
    if (typeof THEMES.current === 'string') {
        body.classList.remove(`dom-theme-${THEMES.current}`);
    }
    THEMES.current = name;
    body.classList.add(`dom-theme-${THEMES.current}`);
}

function getTheme(name: string = "default"): ITheme {
    return THEMES.vars[name] as ITheme;
}

function completeBackgrounds(style: IStyle) {
    let has0 = typeof style.bg0 === 'string';
    const has1 = typeof style.bg1 === 'string';
    const has2 = typeof style.bg2 === 'string';
    let has3 = typeof style.bg3 === 'string';

    if (has0 && has1 && has2 && has3) return

    if (!has0 && !has1 && !has2 && !has3) {
        style.bg0 = "#E0E0E0";
        style.bg1 = "#F5F5F5";
        style.bg2 = "#FAFAFA";
        style.bg3 = "#FFF";
        return;
    }
    if (has0 && !has1 && !has2 && !has3) {
        style.bg3 = lightenBackground(style.bg0 as string);
        has3 = true;
    }
    if (!has0 && !has1 && !has2 && has3) {
        style.bg0 = darkenBackground(style.bg3 as string);
        has3 = true;
    }
    const color0 = new Color(style.bg0);
    const r0 = color0.R;
    const g0 = color0.G;
    const b0 = color0.B;
    const color3 = new Color(style.bg3);
    const r3 = color3.R;
    const g3 = color3.G;
    const b3 = color3.B;
    const color1 = Color.newRGB((2 * r0 + r3) / 3, (2 * g0 + g3) / 3, (2 * b0 + b3) / 3);
    const color2 = Color.newRGB((r0 + 2 * r3) / 3, (g0 + 2 * g3) / 3, (b0 + 2 * b3) / 3);
    style.bg1 = color1.stringify();
    style.bg2 = color2.stringify();
}

function completeWithDefaultValues(style: IStyle) {
    if (typeof style === 'undefined') style = {};

    completeBackgrounds(style);

    if (typeof style.bgP !== 'string') style.bgP = "#3E50B4";
    if (typeof style.bgPD !== 'string') style.bgPD = dark(style.bgP);
    if (typeof style.bgPL !== 'string') style.bgPL = light(style.bgP);
    if (typeof style.bgS !== 'string') style.bgS = rotateHue(style.bgP);
    if (typeof style.bgSD !== 'string') style.bgSD = dark(style.bgS);
    if (typeof style.bgSL !== 'string') style.bgSL = light(style.bgS);

    if (typeof style.white !== 'string') style.white = '#fff';
    if (typeof style.black !== 'string') style.black = '#000';

    THEME_COLOR_NAMES.forEach(function(name) {
        const bg: string = style[`bg${name}`] || '#000';
        COLOR.parse(bg);
        var luminance = COLOR.luminance();
        style[`fg${name}`] = luminance < .6 ? style.white : style.black;
    });

    return style;
}

function dark(color: string) {
    const percent = .25;
    COLOR.parse(color);
    COLOR.rgb2hsl();
    COLOR.L *= 1 - percent;
    COLOR.hsl2rgb();
    return COLOR.stringify();
}

function darkenBackground(color: string) {
    COLOR.parse(color);
    COLOR.rgb2hsl();
    COLOR.L = Math.max(0, COLOR.L - 0.15);
    COLOR.hsl2rgb();
    return COLOR.stringify();
}

function light(color: string) {
    var percent = .4;
    COLOR.parse(color);
    COLOR.rgb2hsl();
    COLOR.L = percent + (1 - percent) * COLOR.L;
    COLOR.hsl2rgb();
    return COLOR.stringify();
}

function lightenBackground(color: string) {
    COLOR.parse(color);
    COLOR.rgb2hsl();
    COLOR.L = Math.min(1, COLOR.L + 0.3);
    COLOR.hsl2rgb();
    return COLOR.stringify();
}

function rotateHue(color: string): string {
    COLOR.parse(color);
    COLOR.rgb2hsl();
    COLOR.H = COLOR.H + .5;
    if (COLOR.H > 1) COLOR.H--;
    COLOR.hsl2rgb();
    return COLOR.stringify();
}

/**
 * @param {string} color - RGB color in format #xxx or #xxxxxx.
 * @param {string} alpha - Single char between 0 and F.
 */
function addAlpha(color: string, alpha: string) {
    if (color.length < 5) return color + alpha;
    return color + alpha + alpha;
}

function removeTail(text: string, tail: string) {
    return text.substr(0, text.length - tail.length);
}

function isDark(colorName: string = ""): boolean {
    if (colorName === "") return isThemeGloballyDark();

    const vars = THEMES.vars[THEMES.current || "default"];
    const varName = `$isDark/${colorName}`;
    let isDark = vars[varName];
    if (typeof isDark === 'boolean') return isDark;

    const color = new Color(vars[colorName]);
    vars[varName] = !color.luminanceStep();
    return vars[varName];
}

function isThemeGloballyDark(): boolean {
    const vars = THEMES.vars[THEMES.current || "default"];
    let isDark = vars.$isDark;
    if (typeof isDark === 'boolean') return isDark;

    const bg0 = new Color(vars.bg0);
    const bg1 = new Color(vars.bg1);
    const bg2 = new Color(vars.bg2);
    const bg3 = new Color(vars.bg3);
    const average = Color.mix(
        Color.mix(bg0, bg1),
        Color.mix(bg2, bg3)
    );
    vars.$isDark = !average.luminanceStep();
    return vars.$isDark;
}

function makeGetCurrentColor(colorName: string): () => string {
    return () => THEMES.vars[THEMES.current || "default"][colorName];
}

function setFontSize(size: "small" | "medium" | "large") {
    const html = document.documentElement;
    html.classList.remove(`thm-font-size-small`);
    html.classList.remove(`thm-font-size-medium`);
    html.classList.remove(`thm-font-size-large`);
    html.classList.add(`thm-font-size-${size}`);
}

function normalize(hexa: string): string {
    const color = new Color(hexa);
    return color.stringify();
}

registerTheme("default", { bgP: "#1e90ff" });

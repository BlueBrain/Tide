const BPE = (new Float32Array()).BYTES_PER_ELEMENT;

/**
 * Creating  a  WebGL  program  for shaders  is  painful.  This  class
 * simplifies the process.
 *
 * @class Program
 *
 * Object properties starting with `$` are WebGL uniforms or attributes.
 * Uniforms behave as expected: you can read/write a value.
 * Attributes when read, return the location. And when written, enable/disabled
 * this attribute. So you read integers and writte booleans.
 *
 * @param gl - WebGL context.
 * @param codes  - Object  with two  mandatory attributes:  `vert` for
 * vertex shader and `frag` for fragment shader.
 * @param  includes  -  (optional)  If  defined,  the  `#include  foo`
 * directives  of  shaders   will  be  replaced  by   the  content  of
 * `includes.foo`.
 */
function Program(gl, codes, includes) {
    if (typeof codes.vert !== 'string') {
        throw Error('[webgl.program] Missing attribute `vert` in argument `codes`!');
    }
    if (typeof codes.frag !== 'string') {
        throw Error('[webgl.program] Missing attribute `frag` in argument `codes`!');
    }

    codes = parseIncludes(codes, includes);

    this.gl = gl;
    Object.freeze(this.gl);
    this.BPE = BPE;
    Object.freeze(this.BPE);

    this._typesNamesLookup = getTypesNamesLookup(gl);

    var shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, getVertexShader(gl, codes.vert));
    gl.attachShader(shaderProgram, getFragmentShader(gl, codes.frag));
    gl.linkProgram(shaderProgram);

    this.program = shaderProgram;
    Object.freeze(this.program);

    this.use = function() {
        gl.useProgram(shaderProgram);
    };
    this.use();

    createAttributes(this, gl, shaderProgram);
    createUniforms(this, gl, shaderProgram);
}

Program.prototype.getTypeName = function(typeId) {
    return this._typesNamesLookup[typeId];
};

Program.prototype.bindAttribs = function(buffer) {
    var gl = this.gl;
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    var names = Array.prototype.slice.call(arguments, 1);
    var totalSize = 0;
    names.forEach(function(name) {
        var attrib = this.attribs[name];
        if (!attrib) {
            throw Error("Cannot find attribute \"" + name + "\"!\n" +
                "It may be not active because unused in the shader.\n" +
                "Available attributes are: " + Object.keys(this.attribs).map(function(name) {
                    return '"' + name + '"';
                }).join(", "));
        }
        totalSize += (attrib.size * attrib.length) * BPE;
    }, this);
    var offset = 0;
    names.forEach(function(name) {
        var attrib = this.attribs[name];
        gl.enableVertexAttribArray(attrib.location);
        gl.vertexAttribPointer(
            attrib.location,
            attrib.size * attrib.length,
            gl.FLOAT,
            false, // No normalisation.
            totalSize,
            offset
        );
        offset += (attrib.size * attrib.length) * BPE;
    }, this);
};

module.exports = Program;


function createAttributes(that, gl, shaderProgram) {
    var index, item;
    var attribs = {};
    var attribsCount = gl.getProgramParameter(shaderProgram, gl.ACTIVE_ATTRIBUTES);
    for (index = 0; index < attribsCount; index++) {
        item = gl.getActiveAttrib(shaderProgram, index);
        item.typeName = that.getTypeName(item.type);
        item.length = getSize.call(that, gl, item);
        item.location = gl.getAttribLocation(shaderProgram, item.name);
        attribs[item.name] = item;
    }

    that.attribs = attribs;
    Object.freeze(that.attribs);
}

function createUniforms(that, gl, shaderProgram) {
    var index, item;
    var uniforms = {};
    var uniformsCount = gl.getProgramParameter(shaderProgram, gl.ACTIVE_UNIFORMS);
    for (index = 0; index < uniformsCount; index++) {
        item = gl.getActiveUniform(shaderProgram, index);
        uniforms[item.name] = gl.getUniformLocation(shaderProgram, item.name);
        Object.defineProperty(that, '$' + item.name, {
            set: createUniformSetter(gl, item, uniforms[item.name], that._typesNamesLookup),
            get: createUniformGetter(item),
            enumerable: true,
            configurable: false
        });
    }
    that.uniforms = uniforms;
    Object.freeze(that.uniforms);
}

/**
 * This is a preprocessor for shaders.
 * Directives  `#include`  will be  replaced  by  the content  of  the
 * correspondent attribute in `includes`.
 */
function parseIncludes(codes, includes) {
    var result = {};
    var id, code;
    for (id in codes) {
        code = codes[id];
        result[id] = code.split('\n').map(function(line) {
            if (line.trim().substr(0, 8) !== '#include') return line;
            var pos = line.indexOf('#include') + 8;
            var includeName = line.substr(pos).trim();
            // We accept all this systaxes:
            // #include foo
            // #include 'foo'
            // #include <foo>
            // #include "foo"
            if ("'<\"".indexOf(includeName.charAt(0)) > -1) {
                includeName = includeName.substr(1, includeName.length - 2);
            }
            var snippet = includes[includeName];
            if (typeof snippet !== 'string') {
                console.error("Include <" + includeName + "> not found in ", includes);
                throw Error("Include not found in shader: " + includeName);
            }
            return snippet;
        }).join("\n");
    }
    return result;
}


function createUniformSetter(gl, item, nameGL, lookup) {
    var nameJS = '_$' + item.name;

    switch (item.type) {
        case gl.BYTE:
        case gl.UNSIGNED_BYTE:
        case gl.SHORT:
        case gl.UNSIGNED_SHORT:
        case gl.INT:
        case gl.UNSIGNED_INT:
        case gl.SAMPLER_2D: // For textures, we specify the texture unit.
            if (item.size === 1) {
                return function(v) {
                    gl.uniform1i(nameGL, v);
                    this[nameJS] = v;
                };
            } else {
                return function(v) {
                    gl.uniform1iv(nameGL, v);
                    this[nameJS] = v;
                };
            }
        case gl.FLOAT:
            if (item.size === 1) {
                return function(v) {
                    gl.uniform1f(nameGL, v);
                    this[nameJS] = v;
                };
            } else {
                return function(v) {
                    gl.uniform1fv(nameGL, v);
                    this[nameJS] = v;
                };
            }
        case gl.FLOAT_VEC2:
            if (item.size === 1) {
                return function(v) {
                    gl.uniform2fv(nameGL, v);
                    this[nameJS] = v;
                };
            } else {
                throw Error(
                    "[webgl.program.createWriter] Don't know how to deal arrays of FLOAT_VEC2 in uniform `" +
                    item.name + "'!'"
                );
            }
        case gl.FLOAT_VEC3:
            if (item.size === 1) {
                return function(v) {
                    gl.uniform3fv(nameGL, v);
                    this[nameJS] = v;
                };
            } else {
                throw Error(
                    "[webgl.program.createWriter] Don't know how to deal arrays of FLOAT_VEC3 in uniform `" +
                    item.name + "'!'"
                );
            }
        case gl.FLOAT_VEC4:
            if (item.size === 1) {
                return function(v) {
                    gl.uniform4fv(nameGL, v);
                    this[nameJS] = v;
                };
            } else {
                throw Error(
                    "[webgl.program.createWriter] Don't know how to deal arrays of FLOAT_VEC4 in uniform `" +
                    item.name + "'!'"
                );
            }
        case gl.FLOAT_MAT3:
            if (item.size === 1) {
                return function(v) {
                    gl.uniformMatrix3fv(nameGL, false, v);
                    this[nameJS] = v;
                };
            } else {
                throw Error(
                    "[webgl.program.createWriter] Don't know how to deal arrays of FLOAT_MAT3 in uniform `" +
                    item.name + "'!'"
                );
            }
        case gl.FLOAT_MAT4:
            if (item.size === 1) {
                return function(v) {
                    gl.uniformMatrix4fv(nameGL, false, v);
                    this[nameJS] = v;
                };
            } else {
                throw Error(
                    "[webgl.program.createWriter] Don't know how to deal arrays of FLOAT_MAT4 in uniform `" +
                    item.name + "'!'"
                );
            }
        default:
            throw Error(
                "[webgl.program.createWriter] Don't know how to deal with uniform `" +
                item.name + "` of type " + lookup[item.type] + "!"
            );
    }
}

function createUniformGetter(item) {
    var name = '_$' + item.name;
    return function() {
        return this[name];
    };
}


function getShader(type, gl, code) {
    var shader = gl.createShader(type);
    gl.shaderSource(shader, code);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        console.log(code);
        console.error("An error occurred compiling the shader: " + gl.getShaderInfoLog(shader));
        return null;
    }

    return shader;
}

function getFragmentShader(gl, code) {
    return getShader(gl.FRAGMENT_SHADER, gl, code);
}

function getVertexShader(gl, code) {
    return getShader(gl.VERTEX_SHADER, gl, code);
}

function getTypesNamesLookup(gl) {
    var lookup = {};
    var k, v;
    for (k in gl) {
        v = gl[k];
        if (typeof v === 'number') {
            lookup[v] = k;
        }
    }
    return lookup;
}

function getSize(gl, item) {
    switch (item.type) {
        case gl.FLOAT_VEC4:
            return 4;
        case gl.FLOAT_VEC3:
            return 3;
        case gl.FLOAT_VEC2:
            return 2;
        case gl.FLOAT:
            return 1;
        default:
            throw Error("[webgl.program:getSize] I don't know the size of the attribute '" + item.name +
                "' because I don't know the type " + this.getTypeName(item.type) + "!");
    }
}

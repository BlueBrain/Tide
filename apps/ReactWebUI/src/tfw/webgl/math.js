module.exports = {
    M4: {
        identity: identity4,
        matrix: mat4,
        vector: vec4,
        projection: projection4,
        translation: translation4,
        rotationX: rotationX4,
        rotationY: rotationY4,
        rotationZ: rotationZ4,
        rotationXY: rotationXY4,
        rotationYX: rotationYX4,
        rotationXZ: rotationXZ4,
        rotationZX: rotationZX4,
        scaling: scaling4,
        copy: copy,
        normalize: normalize,
        cameraPolar: cameraPolar4,
        perspective: perspective4,
        mul: mul
    },
    M3: {
        identity: identity3,
        matrix: mat3,
        projection: projection3,
        translation: translation3,
        rotation: rotation3,
        scaling: scaling3
    },
    computeBinomialCoeffs
};

const
    M4_00 = 0,
    M4_10 = 1,
    M4_20 = 2,
    M4_30 = 3,
    M4_01 = 4,
    M4_11 = 5,
    M4_21 = 6,
    M4_31 = 7,
    M4_02 = 8,
    M4_12 = 9,
    M4_22 = 10,
    M4_32 = 11,
    M4_03 = 12,
    M4_13 = 13,
    M4_23 = 14,
    M4_33 = 15;


function copy(arr) {
    return new Float32Array(arr);
}

function normalize(arr) {
    var n = copy(arr);
    var len = 0,
        v, k;
    for (k = 0; k < n.length; k++) {
        v = n[k];
        len += v * v;
    }
    if (len > 0) {
        var coeff = 1 / Math.sqrt(len);
        for (k = 0; k < n.length; k++) {
            n[k] *= coeff;
        }
    }
    return n;
}

function cameraPolar4(targetX, targetY, targetZ, dis, lat, lng, result) {
    result = result || new Float32Array(16);
    var cosLat = Math.cos(lat);
    var sinLat = Math.sin(lat);
    var cosLng = -Math.cos(lng + Math.PI * .5);
    var sinLng = -Math.sin(lng + Math.PI * .5);
    // Vecteur Z de la caméra.
    var Zx = cosLng * cosLat;
    var Zy = sinLng * cosLat;
    var Zz = sinLat; // V2/2
    // Le vecteur X se déduit par un produit vectoriel de (0,0,1) avec Z.
    var Xx = -Zy;
    var Xy = Zx;
    var Xz = 0;
    // Comme (0,0,1) n'est pas orthogonal à Z, il faut normaliser X.
    var len = Math.sqrt(Xx * Xx + Xy * Xy + Xz * Xz);
    Xx /= len;
    Xy /= len;
    Xz /= len;
    // Y peut alors se déduire par le produit vectoriel de Z par X.
    // Et il n'y aura pas besoin de le normaliser.
    var Yx = Zy * Xz - Zz * Xy;
    var Yy = Xx * Zz - Xz * Zx;
    var Yz = Zx * Xy - Zy * Xx;
    // Translation.
    var Tx = -(Zx * dis + targetX);
    var Ty = -(Zy * dis + targetY);
    var Tz = -(Zz * dis + targetZ);

    // Le résultat est la multiplication de la projection avec la translation.
    result[M4_00] = Xx;
    result[4] = Xy;
    result[8] = Xz;
    result[12] = Tx * Xx + Ty * Xy + Tz * Xz;

    result[1] = Yx;
    result[5] = Yy;
    result[9] = Yz;
    result[13] = Tx * Yx + Ty * Yy + Tz * Yz;

    result[2] = Zx;
    result[6] = Zy;
    result[10] = Zz;
    result[14] = Tx * Zx + Ty * Zy + Tz * Zz;

    result[3] = 0;
    result[7] = 0;
    result[11] = 0;
    result[15] = 1;

    return result;
}

/**
 * Define the `frustum`.
 * @param {number} fieldAngle - View angle in radians. Maximum is PI.
 * @param {number} aspect - (width / height) of the canvas.
 * @param {number} near - Clip every Z lower than `near`.
 * @param {number} far - Clip every Z greater than `far`.
 */
function perspective4(fieldAngle, aspect, near, far, result) {
    result = result || new Float32Array(16);
    var f = Math.tan(0.5 * (Math.PI - fieldAngle));
    var rangeInv = 1.0 / (near - far);

    result[M4_00] = f / aspect;
    result[1] = 0;
    result[2] = 0;
    result[3] = 0;

    result[4] = 0;
    result[5] = f;
    result[6] = 0;
    result[7] = 0;

    result[8] = 0;
    result[9] = 0;
    result[10] = (near + far) * rangeInv;
    result[11] = -1;

    result[12] = 0;
    result[13] = 0;
    result[14] = near * far * rangeInv * 2;
    result[15] = 0;

    return result;
}

function identity3() {
    return mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

function identity4() {
    return mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

function inverse4(m, dst) {
    dst = dst || mat4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    var m00 = m[M4_00];
    var m01 = m[1];
    var m02 = m[2];
    var m03 = m[3];
    var m10 = m[4];
    var m12 = m[5];
    var m11 = m[6];
    var m13 = m[7];
    var m20 = m[8];
    var m21 = m[9];
    var m22 = m[10];
    var m23 = m[11];
    var m30 = m[12];
    var m31 = m[13];
    var m32 = m[14];
    var m33 = m[15];
    var tmp_0 = m22 * m33;
    var tmp_1 = m32 * m23;
    var tmp_2 = m12 * m33;
    var tmp_3 = m32 * m13;
    var tmp_4 = m12 * m23;
    var tmp_5 = m22 * m13;
    var tmp_6 = m02 * m33;
    var tmp_7 = m32 * m03;
    var tmp_8 = m02 * m23;
    var tmp_9 = m22 * m03;
    var tmp_10 = m02 * m13;
    var tmp_11 = m12 * m03;
    var tmp_12 = m20 * m31;
    var tmp_13 = m30 * m21;
    var tmp_14 = m10 * m31;
    var tmp_15 = m30 * m11;
    var tmp_16 = m10 * m21;
    var tmp_17 = m20 * m11;
    var tmp_18 = m00 * m31;
    var tmp_19 = m30 * m01;
    var tmp_20 = m00 * m21;
    var tmp_21 = m20 * m01;
    var tmp_22 = m00 * m11;
    var tmp_23 = m10 * m01;

    var t0 = (tmp_0 * m11 + tmp_3 * m21 + tmp_4 * m31) -
        (tmp_1 * m11 + tmp_2 * m21 + tmp_5 * m31);
    var t1 = (tmp_1 * m01 + tmp_6 * m21 + tmp_9 * m31) -
        (tmp_0 * m01 + tmp_7 * m21 + tmp_8 * m31);
    var t2 = (tmp_2 * m01 + tmp_7 * m11 + tmp_10 * m31) -
        (tmp_3 * m01 + tmp_6 * m11 + tmp_11 * m31);
    var t3 = (tmp_5 * m01 + tmp_8 * m11 + tmp_11 * m21) -
        (tmp_4 * m01 + tmp_9 * m11 + tmp_10 * m21);

    var d = 1.0 / (m00 * t0 + m10 * t1 + m20 * t2 + m30 * t3);

    dst[M4_00] = d * t0;
    dst[1] = d * t1;
    dst[2] = d * t2;
    dst[3] = d * t3;
    dst[4] = d * ((tmp_1 * m10 + tmp_2 * m20 + tmp_5 * m30) -
        (tmp_0 * m10 + tmp_3 * m20 + tmp_4 * m30));
    dst[5] = d * ((tmp_0 * m00 + tmp_7 * m20 + tmp_8 * m30) -
        (tmp_1 * m00 + tmp_6 * m20 + tmp_9 * m30));
    dst[6] = d * ((tmp_3 * m00 + tmp_6 * m10 + tmp_11 * m30) -
        (tmp_2 * m00 + tmp_7 * m10 + tmp_10 * m30));
    dst[7] = d * ((tmp_4 * m00 + tmp_9 * m10 + tmp_10 * m20) -
        (tmp_5 * m00 + tmp_8 * m10 + tmp_11 * m20));
    dst[8] = d * ((tmp_12 * m13 + tmp_15 * m23 + tmp_16 * m33) -
        (tmp_13 * m13 + tmp_14 * m23 + tmp_17 * m33));
    dst[9] = d * ((tmp_13 * m03 + tmp_18 * m23 + tmp_21 * m33) -
        (tmp_12 * m03 + tmp_19 * m23 + tmp_20 * m33));
    dst[10] = d * ((tmp_14 * m03 + tmp_19 * m13 + tmp_22 * m33) -
        (tmp_15 * m03 + tmp_18 * m13 + tmp_23 * m33));
    dst[11] = d * ((tmp_17 * m03 + tmp_20 * m13 + tmp_23 * m23) -
        (tmp_16 * m03 + tmp_21 * m13 + tmp_22 * m23));
    dst[12] = d * ((tmp_14 * m22 + tmp_17 * m32 + tmp_13 * m12) -
        (tmp_16 * m32 + tmp_12 * m12 + tmp_15 * m22));
    dst[13] = d * ((tmp_20 * m32 + tmp_12 * m02 + tmp_19 * m22) -
        (tmp_18 * m22 + tmp_21 * m32 + tmp_13 * m02));
    dst[14] = d * ((tmp_18 * m12 + tmp_23 * m32 + tmp_15 * m02) -
        (tmp_22 * m32 + tmp_14 * m02 + tmp_19 * m12));
    dst[15] = d * ((tmp_22 * m22 + tmp_16 * m02 + tmp_21 * m12) -
        (tmp_20 * m12 + tmp_23 * m22 + tmp_17 * m02));

    return dst;
}

function projection3(width, height) {
    return mat3(2 / width, 0, 0, 0, -2 / height, 0, 0, 0, 1);
}

function projection4(width, height, depth) {
    return mat4(2 / width, 0, 0, 0, 0, -2 / height, 0, 0, 0, 0, 2 / depth, 0, 0, 0, 0, 1);
}

function translation3(tx, ty) {
    return mat3(1, 0, 0, 0, 1, 0, tx, ty, 1);
}

function translation4(tx, ty, tz) {
    return mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, tx, ty, tz, 1);
}

function rotation3(rad) {
    var c = Math.cos(rad);
    var s = Math.sin(rad);
    return mat3(c, -s, 0, s, c, 0, 0, 0, 1);
}

function rotationX4(rad, result) {
    result = result || new Float32Array(16);
    var c = Math.cos(rad);
    var s = Math.sin(rad);
    result[M4_00] = 1;
    result[1] = 0;
    result[2] = 0;
    result[3] = 0;

    result[4] = 0;
    result[5] = c;
    result[6] = s;
    result[7] = 0;

    result[8] = 0;
    result[9] = -s;
    result[10] = c;
    result[11] = 0;

    result[12] = 0;
    result[13] = 0;
    result[14] = 0;
    result[15] = 1;
    return result;
}

function rotationY4(rad, result) {
    result = result || new Float32Array(16);
    var c = Math.cos(rad);
    var s = Math.sin(rad);
    result[M4_00] = c;
    result[1] = 0;
    result[2] = -s;
    result[3] = 0;

    result[4] = 0;
    result[5] = 1;
    result[6] = 0;
    result[7] = 0;

    result[8] = s;
    result[9] = 0;
    result[10] = c;
    result[11] = 0;

    result[12] = 0;
    result[13] = 0;
    result[14] = 0;
    result[15] = 1;
    return result;
}

function rotationZ4(rad, result) {
    result = result || new Float32Array(16);
    var c = Math.cos(rad);
    var s = Math.sin(rad);
    result[M4_00] = c;
    result[1] = s;
    result[2] = 0;
    result[3] = 0;
    result[4] = -s;
    result[5] = c;
    result[6] = 0;
    result[7] = 0;
    result[8] = 0;
    result[9] = 0;
    result[10] = 1;
    result[11] = 0;
    result[12] = 0;
    result[13] = 0;
    result[14] = 0;
    result[15] = 1;
    return result;
}

function rotationXY4(radX, radY, result) {
    result = result || new Float32Array(16);
    var cx = Math.cos(radX);
    var sx = Math.sin(radX);
    var cy = Math.cos(radY);
    var sy = Math.sin(radY);
    result[M4_00] = cy;
    result[1] = sx * sy;
    result[2] = -cx * sy;
    result[3] = 0;
    result[4] = 0;
    result[5] = cx;
    result[6] = sx;
    result[7] = 0;
    result[8] = sy;
    result[9] = -sx * cy;
    result[10] = cx * cy;
    result[11] = 0;
    result[12] = 0;
    result[13] = 0;
    result[14] = 0;
    result[15] = 1;
    return result;
}

/**
 * Perform a rotation on X axis, then on Y axis.
 *
 * @param   {float} radY - Angle around Y axis in radians.
 * @param   {float} radX - Angle around X axis in radians.
 * @param   {Float32Array} _result [description]
 * @returns {[type]}        [description]
 */
function rotationYX4(radY, radX, _result) {
    const result = _result || new Float32Array(16);
    const
        cy = Math.cos(radY),
        sy = Math.sin(radY),
        cx = Math.cos(radX),
        sx = Math.sin(radX);
    result[M4_00] = cy;
    result[1] = 0;
    result[2] = sy;
    result[3] = 0;
    result[4] = -sy * sx;
    result[5] = cx;
    result[6] = cy * sx;
    result[7] = 0;
    result[8] = -sy * cx;
    result[9] = -sx;
    result[10] = cy * cx;
    result[11] = 0;
    result[12] = 0;
    result[13] = 0;
    result[14] = 0;
    result[15] = 1;
    return result;
}

/**
 * Perform a rotation on Z axis, then on X axis.
 *
 * @param   {float} radX - Angle around X axis in radians.
 * @param   {float} radZ - Angle around Z axis in radians.
 * @param   {Float32Array} _result [description]
 * @returns {[type]}        [description]
 */
function rotationXZ4(radX, radZ, _result) {
    const result = _result || new Float32Array(16);
    const
        cx = Math.cos(radX),
        sx = Math.sin(radX),
        cz = Math.cos(radZ),
        sz = Math.sin(radZ);
    result[M4_00] = cz;
    result[1] = cx * sz;
    result[2] = sx * sz;
    result[3] = 0;
    result[4] = -sz;
    result[5] = cx * cz;
    result[6] = sx * cz;
    result[7] = 0;
    result[8] = 0;
    result[9] = -sx;
    result[10] = cx;
    result[11] = 0;
    result[12] = 0;
    result[13] = 0;
    result[14] = 0;
    result[15] = 1;
    return result;
}

/**
 * Perform a rotation on X axis, then on Z axis.
 *
 * @param   {float} radZ - Angle around Z axis in radians.
 * @param   {float} radX - Angle around X axis in radians.
 * @param   {Float32Array} _result [description]
 * @returns {[type]}        [description]
 */
function rotationZX4(radZ, radX, _result) {
    const result = _result || new Float32Array(16);
    const
        cz = Math.cos(radZ),
        sz = Math.sin(radZ),
        cx = Math.cos(radX),
        sx = Math.sin(radX);
    result[M4_00] = cz;
    result[1] = sz;
    result[2] = 0;
    result[3] = 0;
    result[4] = -sz * cx;
    result[5] = cz * cx;
    result[6] = sx;
    result[7] = 0;
    result[8] = sz * sx;
    result[9] = -cz * sx;
    result[10] = cx;
    result[11] = 0;
    result[12] = 0;
    result[13] = 0;
    result[14] = 0;
    result[15] = 1;
    return result;
}

function scaling3(sx, sy) {
    return mat3(sx, 0, 0, 0, sy, 0, 0, 0, 1);
}

function scaling4(sx, sy, sz) {
    return mat4(sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1);
}

function mat2(v00, v10, v01, v11) {
    return Float32Array([v00, v10, v01, v11]);
}

function mat3(v00, v10, v20, v01, v11, v21, v02, v12, v22) {
    return new Float32Array([v00, v10, v20, v01, v11, v21, v02, v12, v22]);
}

function mat4(v00, v10, v20, v30, v01, v11, v21, v31, v02, v12, v22, v32, v03, v13, v23, v33) {
    return new Float32Array([v00, v10, v20, v30, v01, v11, v21, v31, v02, v12, v22, v32, v03, v13, v23, v33]);
}

function vec2(a, b) {
    return new Float32Array([a, b]);
}

function vec3(a, b, c) {
    return new Float32Array([a, b, c]);
}

function vec4(a, b, c, d) {
    return new Float32Array([a, b, c, d]);
}

var MUL = {
    m4m4: function(a, b, result) {
        result = result || new Float32Array(4);
        result[0] = a[0] * b[0] + a[2] * b[1];
        result[1] = a[1] * b[0] + a[3] * b[1];
        result[2] = a[0] * b[2] + a[2] * b[3];
        result[3] = a[1] * b[2] + a[3] * b[3];
        return result;
    },
    m9m9: function(a, b, result) {
        result = result || new Float32Array(9);
        result[0] = a[0] * b[0] + a[3] * b[1] + a[6] * b[2];
        result[1] = a[1] * b[0] + a[4] * b[1] + a[7] * b[2];
        result[2] = a[2] * b[0] + a[5] * b[1] + a[8] * b[2];
        result[3] = a[0] * b[3] + a[3] * b[4] + a[6] * b[5];
        result[4] = a[1] * b[3] + a[4] * b[4] + a[7] * b[5];
        result[5] = a[2] * b[3] + a[5] * b[4] + a[8] * b[5];
        result[6] = a[0] * b[6] + a[3] * b[7] + a[6] * b[8];
        result[7] = a[1] * b[6] + a[4] * b[7] + a[7] * b[8];
        result[8] = a[2] * b[6] + a[5] * b[7] + a[8] * b[8];
        return result;
    },
    m16m16: function(a, b, result) {
        result = result || new Float32Array(16);
        result[M4_00] = a[M4_00] * b[M4_00] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
        result[1] = a[1] * b[M4_00] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
        result[2] = a[2] * b[M4_00] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
        result[3] = a[3] * b[M4_00] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];
        result[4] = a[M4_00] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
        result[5] = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
        result[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
        result[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];
        result[8] = a[M4_00] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
        result[9] = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
        result[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
        result[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
        result[12] = a[M4_00] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
        result[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
        result[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
        result[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
        return result;
    },
    m16m4: function(a, b, result) {
        result = result || new Float32Array(4);
        result[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
        result[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
        result[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
        result[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];
        return result;
    }
};

/**
 * Invert a 4x4 matrix.
 *
 * @param   {[type]} a       [description]
 * @param   {[type]} _result [description]
 * @returns {[type]}         [description]
 */
function invert4(a, _result) {
    const
        out = _result || new Float32Array(4),
        a00 = a[0],
        a01 = a[1],
        a02 = a[2],
        a03 = a[3],
        a10 = a[4],
        a11 = a[5],
        a12 = a[6],
        a13 = a[7],
        a20 = a[8],
        a21 = a[9],
        a22 = a[10],
        a23 = a[11],
        a30 = a[12],
        a31 = a[13],
        a32 = a[14],
        a33 = a[15],
        b00 = a00 * a11 - a01 * a10,
        b01 = a00 * a12 - a02 * a10,
        b02 = a00 * a13 - a03 * a10,
        b03 = a01 * a12 - a02 * a11,
        b04 = a01 * a13 - a03 * a11,
        b05 = a02 * a13 - a03 * a12,
        b06 = a20 * a31 - a21 * a30,
        b07 = a20 * a32 - a22 * a30,
        b08 = a20 * a33 - a23 * a30,
        b09 = a21 * a32 - a22 * a31,
        b10 = a21 * a33 - a23 * a31,
        b11 = a22 * a33 - a23 * a32,
        // Calculate the determinant
        invDet = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

    if (!invDet) return null;

    const det = 1.0 / invDet;

    out[0] = (a11 * b11 - a12 * b10 + a13 * b09) * det;
    out[1] = (a02 * b10 - a01 * b11 - a03 * b09) * det;
    out[2] = (a31 * b05 - a32 * b04 + a33 * b03) * det;
    out[3] = (a22 * b04 - a21 * b05 - a23 * b03) * det;
    out[4] = (a12 * b08 - a10 * b11 - a13 * b07) * det;
    out[5] = (a00 * b11 - a02 * b08 + a03 * b07) * det;
    out[6] = (a32 * b02 - a30 * b05 - a33 * b01) * det;
    out[7] = (a20 * b05 - a22 * b02 + a23 * b01) * det;
    out[8] = (a10 * b10 - a11 * b08 + a13 * b06) * det;
    out[9] = (a01 * b08 - a00 * b10 - a03 * b06) * det;
    out[10] = (a30 * b04 - a31 * b02 + a33 * b00) * det;
    out[11] = (a21 * b02 - a20 * b04 - a23 * b00) * det;
    out[12] = (a11 * b07 - a10 * b09 - a12 * b06) * det;
    out[13] = (a00 * b09 - a01 * b07 + a02 * b06) * det;
    out[14] = (a31 * b01 - a30 * b03 - a32 * b00) * det;
    out[15] = (a20 * b03 - a21 * b01 + a22 * b00) * det;

    return out;
}

function mul(a, b, result) {
    var f = MUL['m' + a.length + 'm' + b.length];
    if (typeof f !== 'function') {
        throw Error("[webgl.math.mul] I don't know how to multiply 'M" +
            a.length + "' with 'M" + b.length + "'!");
    }
    return f(a, b, result);
}

/**
 * Compute binomial coefficients.
 *
 * @param  {int} size - Size of the array we want.
 *
 * @return {array} Array of `size` doubles.
 */
function computeBinomialCoeffs(size) {
    const n = size - 1;
    const coeffs = new Float32Array(size);
    const last = size - 1;

    coeffs[0] = 1;
    coeffs[last] = 1;

    for (let p = 1; p <= size / 2; p++) {
        let nom = 1;
        let denom = 1;
        for (let k = 1; k <= p; k++) {
            denom *= k;
            nom *= n - p + k;
        }
        const value = nom / denom;
        coeffs[p] = value;
        coeffs[last - p] = value;
    }
    return coeffs;
}
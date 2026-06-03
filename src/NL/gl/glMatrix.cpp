#include "NL/gl/glMatrix.h"
#include "NL/glx/glxMatrix.h"
#include "NL/glx/glxMemory.h"

#include "NL/platvmath.h"
#include <string.h>

static unsigned long gl_IdentityMatrix = 0xFFFFFFFF;

/**
 * Offset/Address/Size: 0x0 | 0x801D8A74 | size: 0x20
 */
void glMatrixLookAt(nlMatrix4& matrix, const nlVector3& eyePos, const nlVector3& targetPos, const nlVector3& upVector)
{
    glplatMatrixLookAt(matrix, eyePos, targetPos, upVector);
}

/**
 * Offset/Address/Size: 0x20 | 0x801D8A94 | size: 0x20
 */
void glMatrixPerspective(nlMatrix4& matrix, float fovY, float aspect, float near, float far)
{
    glplatMatrixPerspective(matrix, fovY, aspect, near, far);
}

/**
 * Offset/Address/Size: 0x40 | 0x801D8AB4 | size: 0x20
 */
void glMatrixOrthographicCentered(nlMatrix4& matrix, float width, float height, float near, float far)
{
    glplatMatrixOrthographicCentered(matrix, width, height, near, far);
}

/**
 * Offset/Address/Size: 0x60 | 0x801D8AD4 | size: 0x20
 */
void glMatrixOrthographic(nlMatrix4& matrix, float width, float height)
{
    glplatMatrixOrthographic(matrix, width, height);
}

/**
 * Offset/Address/Size: 0x80 | 0x801D8AF4 | size: 0x20
 */
void glSetMatrix(unsigned long arg0, const nlMatrix4& matrix)
{
    glplatSetMatrix(arg0, matrix);
}

/**
 * Offset/Address/Size: 0xA0 | 0x801D8B14 | size: 0x20
 */
void glGetMatrix(unsigned long arg0, nlMatrix4& matrix)
{
    glplatGetMatrix(arg0, matrix);
}

/**
 * Offset/Address/Size: 0xC0 | 0x801D8B34 | size: 0x34
 */
u32 glAllocMatrix()
{
    u32 var_r3 = glplatFrameAlloc(0x40, (eGLMemory)1);
    if (var_r3 == 0U)
    {
        var_r3 = -1U;
    }
    return var_r3;
}

/**
 * Offset/Address/Size: 0xF4 | 0x801D8B68 | size: 0x8
 */
unsigned long glGetIdentityMatrix()
{
    return gl_IdentityMatrix; // probably a pointer to a matrix
}

/**
 * Offset/Address/Size: 0xFC | 0x801D8B70 | size: 0x3C
 */
void gl_MatrixStartup()
{
    nlMatrix4 matrix;
    matrix.SetIdentity();
    gl_IdentityMatrix = (unsigned long)glplatResourceAlloc(0x40, (eGLMemory)1);
    glplatSetMatrix(gl_IdentityMatrix, matrix);
}

/**
 * Offset/Address/Size: 0x138 | 0x801D8BAC | size: 0x24
 */
void GLMatrix::Set(const nlMatrix4& matrix)
{
    memcpy(&m_matrix, &matrix, 0x40);
}

/**
 * Offset/Address/Size: 0x15C | 0x801D8BD0 | size: 0x30
 */
void GLMatrix::Get(nlMatrix4& matrix) const
{
    memcpy(&matrix, &m_matrix, 0x40);
}

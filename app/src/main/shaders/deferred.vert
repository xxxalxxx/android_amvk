#version 450 
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 outUV;
 
void main() {

	// Vertex Indices are in clockwise order
	//
	//  [0]-----[1]
	//    \      |
	//     \     |
	//      \    |
	//       \   |
	//        \  |
	//         \ |
	//          \|
	//          [2]
	//
	// [0] -> (0 & 2, 0 & 2) -> (0, 0) -> (-1, -1)
	// [1] -> (2 & 2, 1 & 2) -> (2, 0) -> ( 3, -1)
	// [2] -> (4 & 2, 2 & 2) -> (2, 2) -> ( 3,  3)

	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}

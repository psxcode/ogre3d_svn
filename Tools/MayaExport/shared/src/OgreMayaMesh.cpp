/*
===============================================================================
This source file is part of the Ogre-Maya Tools.
Distributed as part of Ogre (Object-oriented Graphics Rendering Engine).
Copyright � 2003 Fifty1 Software Inc. 

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
===============================================================================
*/

#include "OgreMayaMesh.h"
#include "OgreMayaOptions.h"

#include <maya/MItGeometry.h>
#include <maya/MFnMesh.h>
#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnReflectShader.h>
#include <maya/MFnSet.h>
#include <maya/MPlugArray.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItDag.h>

#include <maya/MItDependencyNodes.h>
#include <maya/MFnSkinCluster.h>

#include <iostream.h>
#include <string>

namespace OgreMaya {

    using namespace std;

	//	--------------------------------------------------------------------------
	/** Standard constructor. Creates Ogre Mesh and defines known options.
	*/	
	//	--------------------------------------------------------------------------
	MeshGenerator::MeshGenerator() {
	}


	//	--------------------------------------------------------------------------
	/** Destructor.
	*/	
	//	--------------------------------------------------------------------------
	MeshGenerator::~MeshGenerator()
	{
	}


	//	--------------------------------------------------------------------------
	/** Finds and exports all polygonal meshes in the DAG. Each polygonal mesh
		corresponds to a single Ogre SubMesh.

		\return		True if exported ok, false otherwise
	*/	
	//	--------------------------------------------------------------------------
	bool MeshGenerator::exportAll()
	{
		MStatus status;
		bool bStatus = true;        

		// ===== Iterate over mesh components of DAG

		// --- Setup iterator
		MItDag iterDag(MItDag::kDepthFirst, MFn::kMesh, &status);
		if (status == MStatus::kFailure) {
			MGlobal::displayError("MItDag::MItDag");
			return false;
		}


        {
            ofstream out(OPTIONS.outMeshFile.c_str());

            out.precision(5);
            out.setf(ios::fixed);
        
            out << "<mesh>\n";
            out << "\t<submeshes>\n";

		    // --- Iterate
		    for(; !iterDag.isDone(); iterDag.next()) {
		    
			    // Get DAG path
			    MDagPath dagPath;
			    status = iterDag.getPath(dagPath);
			    if (status == MStatus::kFailure) {
				    MGlobal::displayError("MDagPath::getPath");
				    bStatus = false;
				    break;
			    }

			    // Process this node?
			    if ( dagPath.hasFn(MFn::kTransform)) continue;
			    if (!dagPath.hasFn(MFn::kMesh))      continue;
			    MFnDagNode dagNode(dagPath);
			    if (dagNode.isIntermediateObject())  continue;
			    
			    // Process node if visible
			    bool bVisible;
			    bVisible = _isVisible(dagNode, status);
			    if (bVisible && (status == MStatus::kSuccess)) {
				    status = _processPolyMesh(out, dagPath);
				    if (status != MStatus::kSuccess) {
					    break;
				    }
			    }
		    }

            out << "\t</submeshes>\n";
            
            if(OPTIONS.exportSkeleton) {
                string skeletonName =
                    OPTIONS.outSkelFile.substr(
                        0, OPTIONS.outSkelFile.find_last_of('.')
                    );
                out << "\t<skeletonlink name=\""<<skeletonName<<"\"/>\n";
            }

            out << "</mesh>\n";
        }

		
		// ===== Done
		return (status == MStatus::kSuccess);
	}


	//	--------------------------------------------------------------------------
	/**	Process a Maya polyMesh to generate an Ogre SubMesh.

		\param		dagPath
					Path to the Maya polyMesh to be processed

		\return		MStatus::kSuccess if processed successfuly, 
					different MStatus otherwise

		\todo		Vertex optimization
		\todo		Submesh optimization (merge submeshes that share materials)
	*/	
	//	--------------------------------------------------------------------------
	MStatus MeshGenerator::_processPolyMesh(ofstream& out, const MDagPath dagPath) {
		MStatus status = MStatus::kSuccess;
        MeshMayaGeometry MayaGeometry;

//*******************************************************************

        // ===== Calculate the influence of SkinCluter if any
        bool hasSkinCluster = false;                    

        
        MObject	kInputObject, kOutputObject;
        MDagPath meshPath = dagPath;        
        MFnMesh fnTargetMesh(dagPath, &status);
            
	    //search the skin cluster affecting this geometry
	    MItDependencyNodes kDepNodeIt( MFn::kSkinClusterFilter );            

	    for( ;!kDepNodeIt.isDone() && !hasSkinCluster; kDepNodeIt.next())  {
		    MObject kObject = kDepNodeIt.item();

		    MFnSkinCluster kSkinClusterFn(kObject, &status);

            cout << kSkinClusterFn.name().asChar() << '\n';


            unsigned int uiNumGeometries = kSkinClusterFn.numOutputConnections();

            cout << "  [" << uiNumGeometries << "] geometry objects\n";

            for(unsigned int uiGeometry = 0; uiGeometry < uiNumGeometries; ++uiGeometry ) {
	            unsigned int uiIndex = kSkinClusterFn.indexForOutputConnection( uiGeometry, &status );


	            kInputObject = kSkinClusterFn.inputShapeAtIndex( uiIndex, &status );
	            kOutputObject = kSkinClusterFn.outputShapeAtIndex( uiIndex, &status );

                if(kOutputObject == fnTargetMesh.object()) {
                    cout << "geometry found in skin cluster\n";
                    hasSkinCluster = true;


                    // get weights
                    MItGeometry kGeometryIt(kInputObject);
			        for(int uiVertex = 0; !kGeometryIt.isDone(); kGeometryIt.next(), ++uiVertex ) {
				        MObject kComponent = kGeometryIt.component( 0 );

				        MFloatArray kWeightArray;
				        unsigned int uiNumInfluences;

				        kSkinClusterFn.getWeights(meshPath, kComponent, kWeightArray, uiNumInfluences);
                                    
                        MayaGeometry.Weights.push_back(kWeightArray);
                    }
                }
            }
        
        }

        MFnMesh fnMesh = hasSkinCluster ? MFnMesh(kInputObject) : MFnMesh(dagPath, &status);
        
//*******************************************************************




        // ===== Get Maya geometry		
		status = _queryMayaGeometry(fnMesh, MayaGeometry);
		if (status == MStatus::kFailure) {
			return status;
		}


		// ===== Parse into MeshGenerator format
		MeshFaceVertexVector FaceVertices;
		MeshTriFaceList      TriFaces;
		status = _parseMayaGeometry(fnMesh, MayaGeometry, FaceVertices, TriFaces);
		if (status == MStatus::kFailure) {
			return status;
		}

		
		// ===== Optimize Mesh
        // TODO
		//_optimizeMesh(FaceVertices, TriFaces);



////////////////////////////////////////////////////////////////////////////////


        // export as xml

        out << "\t\t<submesh material=\"" << MayaGeometry.MaterialName.asChar() << "\" usesharedvertices=\"false\" use32bitindexes=\"false\">\n";    
	

		// ===== Create Ogre face list
		// --- Basic info
		unsigned int nTriFaces = (unsigned int)TriFaces.size();
		// --- Store face indices
		MeshTriFaceList::iterator faceIt, faceEnd;
        faceEnd = TriFaces.end();

        out << "\t\t\t<faces count=\"" << nTriFaces << "\">\n";
        for(faceIt=TriFaces.begin(); faceIt!=faceEnd; ++faceIt) {
            out << "\t\t\t\t<face ";
			out << "v1=\"" << faceIt->index0 << "\" ";
			out << "v2=\"" << faceIt->index1 << "\" ";
			out << "v3=\"" << faceIt->index2 << "\"/>\n";
		}
        out << "\t\t\t</faces>\n";



        out << "\t\t\t<geometry vertexcount=\"" << FaceVertices.size() << "\">\n";
        

		MeshFaceVertexVector::iterator vertexIt, vertexEnd;
        vertexEnd = FaceVertices.end();

        //
        // POSITIONS
        //
        out << "\t\t\t\t<vertexbuffer positions=\"true\">\n";	    
        
		for(vertexIt=FaceVertices.begin(); vertexIt!=vertexEnd; ++vertexIt)	{            
            out << "\t\t\t\t\t<vertex>\n";
            out << "\t\t\t\t\t\t<position ";
            out << "x=\"" << vertexIt->vecPosition.x*100 << "\" ";
			out << "y=\"" << vertexIt->vecPosition.y*100 << "\" ";
			out << "z=\"" << vertexIt->vecPosition.z*100 << "\"/>\n";
            out << "\t\t\t\t\t</vertex>\n";
		}
		out << "\t\t\t\t</vertexbuffer>\n";

        //
        // NORMALS
        //
        if(OPTIONS.exportNormals) {
            out << "\t\t\t\t<vertexbuffer normals=\"true\">\n";
        
		    for(vertexIt=FaceVertices.begin(); vertexIt!=vertexEnd; ++vertexIt)	{            
                out << "\t\t\t\t\t<vertex>\n";            
                out << "\t\t\t\t\t\t<normal ";
                out << "x=\"" << vertexIt->vecNormal.x << "\" ";
			    out << "y=\"" << vertexIt->vecNormal.y << "\" ";
                out << "z=\"" << vertexIt->vecNormal.z << "\"/>\n";
                out << "\t\t\t\t\t</vertex>\n";
		    }
		    out << "\t\t\t\t</vertexbuffer>\n";
        }

        //
        // COLORS
        //
        if(OPTIONS.exportColours) {
            out << "\t\t\t\t<vertexbuffer colours_diffuse=\"true\">\n";

            for(vertexIt=FaceVertices.begin(); vertexIt!=vertexEnd; ++vertexIt)	{            
                out << "\t\t\t\t\t<vertex>\n";
				//vertexIt->colour.getAsLongRGBA();
                out << "\t\t\t\t\t\t<colour_diffuse value=\"";
                out << vertexIt->colour.r << " ";
			    out << vertexIt->colour.g << " ";
                out << vertexIt->colour.b << " ";
			    out << vertexIt->colour.a << "\"/>\n";                
                out << "\t\t\t\t\t</vertex>\n";
		    }
		    out << "\t\t\t\t</vertexbuffer>\n";
        }


        //
        // TEXCORDS
        //
        if(MayaGeometry.UVSets.size() > 0) {
            out << "\t\t\t\t<vertexbuffer ";
            out << "texture_coords=\"" << MayaGeometry.UVSets.size() << "\"";
            out << ">\n";

            for(vertexIt=FaceVertices.begin(); vertexIt!=vertexEnd; ++vertexIt)	{            
                out << "\t\t\t\t\t<vertex>\n";
				MeshVertexUVList::iterator uvIt, uvEnd;
				uvEnd = vertexIt->listUV.end();
				for (uvIt = vertexIt->listUV.begin(); uvIt!=uvEnd; ++uvIt) {                    
                    out << "\t\t\t\t\t\t<texcoord ";
                    out << "u=\"" << uvIt->u << "\" ";
			        out << "v=\"" << uvIt->v << "\"/>\n";
				}

                out << "\t\t\t\t\t</vertex>\n";
		    }
		    out << "\t\t\t\t</vertexbuffer>\n";
        }

        out << "\t\t\t</geometry>\n";


        // BONE ASSIGMENTS
        if(OPTIONS.exportSkeleton) {
            out << "\t\t\t<boneassignments>\n";

            for(vertexIt=FaceVertices.begin(); vertexIt!=vertexEnd; ++vertexIt)	{            
                VertexBoneAssigmentList::iterator boneIt, boneEnd;
			    boneEnd = vertexIt->boneAssigments.end();
			    for (boneIt=vertexIt->boneAssigments.begin(); boneIt!=boneEnd; ++boneIt) {
                    VertexBoneAssigment& assigment = *boneIt;

                    if(assigment.weight<0.01)
                        continue;

                    out << "\t\t\t\t<vertexboneassignment ";
                    out << "vertexindex=\""<<assigment.vertexId<<"\" ";
                    out << "boneindex=\""<<assigment.boneId<<"\" ";
                    out << "weight=\""<<assigment.weight<<"\"/>\n";           
                }
		    }
        

            out << "\t\t\t</boneassignments>\n";
        }


        out << "\t\t</submesh>\n";

////////////////////////////////////////////////////////////////////////////////


		// ===== Success!
		return MStatus::kSuccess;
	}

    MString getMaterialName(MFnMesh &fnMesh) {
		MStatus status = MStatus::kSuccess;
		MString MaterialName = "";
		

		// ===== Connected sets and members
		// (Required to determine texturing of different faces)

		// Determine instance number
		fnMesh.dagPath().extendToShape();
		int iInstance = 0;
		if (fnMesh.dagPath().isInstanced()) {
			iInstance = fnMesh.dagPath().instanceNumber();
		}

		// Get the connected sets and members
		MObjectArray PolygonSets;
		MObjectArray PolygonComponents;
		status = fnMesh.getConnectedSetsAndMembers(iInstance, 
			                                       PolygonSets, 
												   PolygonComponents, 
												   true);
		if (!status) {
			MGlobal::displayError("MFnMesh::getConnectedSetsAndMembers"); 
			return MaterialName;
		}


		// ===== Materials
		unsigned int iSet;
		for (iSet = 0; iSet < PolygonSets.length(); ++iSet) {
			MObject PolygonSet = PolygonSets[iSet];
			MObject PolygonComponent = PolygonComponents[iSet];

			MFnDependencyNode dnSet(PolygonSet);
			MObject ssAttr = dnSet.attribute(MString("surfaceShader"));
			MPlug ssPlug(PolygonSet, ssAttr);

			MPlugArray srcPlugArray;
			ssPlug.connectedTo(srcPlugArray, true, false);
			
			if (srcPlugArray.length() > 0) {
				// This object contains a reference to a shader or material.
				// Check for known material types and extract material name.
				MObject srcNode = srcPlugArray[0].node();
				
				if (srcNode.hasFn(MFn::kPhong)) {
					MFnPhongShader fnPhong(srcNode);
					MaterialName = fnPhong.name();
                }
				else if (srcNode.hasFn(MFn::kLambert)) {
					MFnLambertShader fnLambert(srcNode);
					MaterialName = fnLambert.name();
				}
				else if (srcNode.hasFn(MFn::kBlinn)) {
					MFnBlinnShader fnBlinn(srcNode);
					MaterialName = fnBlinn.name();
				}
				else if (srcNode.hasFn(MFn::kReflect)) {
					MFnReflectShader fnReflect(srcNode);
					MaterialName = fnReflect.name();
				}
            }
		}


		// ===== Done
		return MaterialName;

	}

	//	--------------------------------------------------------------------------
	/**	Retrieve all Maya geometry for a single Maya mesh.
		\todo		Define materials if requested
		\todo		Fix normals
	*/	
	//	--------------------------------------------------------------------------
	MStatus MeshGenerator::_queryMayaGeometry(MFnMesh &fnMesh, 
		                                      MeshMayaGeometry &rGeom)
	{
		MStatus status = MStatus::kSuccess;


		// ===== Identification
		rGeom.Name         = fnMesh.partialPathName(); // shortest unique name
		rGeom.MaterialName = getMaterialName(fnMesh);


		// ===== Geometry

		// --- Vertices
		status = fnMesh.getPoints(rGeom.Vertices, MSpace::kWorld);
		if (status == MStatus::kFailure) {
			MGlobal::displayError("MFnMesh::getPoints()"); 
			return status;
		}

        cout << "vertices count: " << rGeom.Vertices.length() << '\n';

		// --- Vertex normals
		status = fnMesh.getNormals(rGeom.FaceVertexNormals);
		if (status == MStatus::kFailure) {
			MGlobal::displayError("MFnMesh::getNormals()"); 
			return status;
		}

		// --- Triangular faces
		MDagPath dagPath;
		fnMesh.getPath(dagPath);
		MItMeshPolygon iterPoly(dagPath);
		
		int iPolygon, nPolygons;
		nPolygons = fnMesh.numPolygons();
		for (iPolygon=0; iPolygon < nPolygons; ++iPolygon)
		{
			MIntArray ThisPolyTriVertices;
			MPointArray ThisPolyPointsUntweaked;
			iterPoly.getTriangles(ThisPolyPointsUntweaked, ThisPolyTriVertices, MSpace::kWorld);

			_convertObjectToFace(iterPoly, ThisPolyTriVertices, rGeom.TriangleVertexIds);

			int iTriangle, nTriangles;
			iterPoly.numTriangles(nTriangles);
			for (iTriangle=0; iTriangle < nTriangles; ++iTriangle) {
				rGeom.TrianglePolygonIds.append(iPolygon);
			}

			iterPoly.next();
		}


		// ===== Colours and UVs

		// --- Face vertex colours
		status = fnMesh.getFaceVertexColors(rGeom.FaceVertexColours);
		if (status == MStatus::kFailure) {
			MGlobal::displayError("MFnMesh::getFaceVertexColors()"); 
			return status;
		}
		// Override non-existent colours with semi-transparent white
		unsigned int iFaceVertex;
		MColor mayaColour;
		for (iFaceVertex=0; iFaceVertex < rGeom.FaceVertexColours.length(); ++iFaceVertex) 
		{
			mayaColour = rGeom.FaceVertexColours[iFaceVertex];
			if ((mayaColour.r) == -1) mayaColour.r = 1;
			if ((mayaColour.g) == -1) mayaColour.g = 1;
			if ((mayaColour.b) == -1) mayaColour.b = 1;
			if ((mayaColour.a) == -1) mayaColour.a = 0.2;
			rGeom.FaceVertexColours[iFaceVertex] = mayaColour;
		}

		// --- UV set names
		MStringArray UVSetNames;
		status = fnMesh.getUVSetNames(UVSetNames);
		if (status == MStatus::kFailure) {
			MGlobal::displayError("MFnMesh::getUVSetNames()"); 
			return status;
		}

		// --- Linked list of UV sets
		unsigned int nUVSets = UVSetNames.length();
		unsigned int iUVSet;

		// Loop over all UV sets
		MeshMayaUVSet UVSet;
		for (iUVSet = 0; iUVSet < nUVSets; ++iUVSet) {

			// Store UV name
			UVSet.sName = UVSetNames[iUVSet];

			// Retrieve UV coordinates
			status = fnMesh.getUVs(UVSet.uArray, UVSet.vArray, &(UVSet.sName));
			if (status == MStatus::kFailure) {
				return status;
			}

			// Store UV set
			rGeom.UVSets.push_back(UVSet);
		}


		// ===== Done
		return status;

	}


	//	--------------------------------------------------------------------------
	/** Parse Maya geometry into MeshGenerator format for further processing.
	*/	
	//	--------------------------------------------------------------------------
	MStatus MeshGenerator::_parseMayaGeometry(MFnMesh &fnMesh,
		                                      MeshMayaGeometry &MayaGeometry, 
		                                      MeshFaceVertexVector &FaceVertices, 
			    			                  MeshTriFaceList &TriFaces)
	{
		MStatus status;

		// --- Determine number of triangles
		unsigned int nTris = MayaGeometry.TrianglePolygonIds.length();
		if (nTris == 0) {
			return MStatus::kFailure;
		}

		// --- Confirm number of triangle vertices
		unsigned int nTriVertices = MayaGeometry.TriangleVertexIds.length();
		if (nTriVertices != 3*nTris) {
			cout << "MeshGenerator: "<<nTris<<" triangles require "<<(3*nTris)<<" vertices but "<<nTriVertices<<" vertices present!\n";
			return MStatus::kFailure;
		}

		// --- Loop over all triangles
		unsigned int iTri;
		cout << "MeshGenerator: Exporting "<<fnMesh.numPolygons()<<" faces as "<<nTris<<" triangles from "<<MayaGeometry.Name.asChar()<<" (material "<<MayaGeometry.MaterialName.asChar()<<")...\n";

		for (iTri = 0; iTri < nTris; ++iTri) {

			// --- Get polygon index
			unsigned int iPoly;
			iPoly = MayaGeometry.TrianglePolygonIds[iTri];

			// --- Get indices of face-vertices
			MIntArray VertexIds;
			status = fnMesh.getPolygonVertices(iPoly, VertexIds);
			if (status == MStatus::kFailure) {
				MGlobal::displayError("MFnMesh::getPolygonVertices()");
				return status;
			}

			// --- Get indices of face-vertex normals
			MIntArray NormalIds;
			if (OPTIONS.exportNormals) {
				fnMesh.getFaceNormalIds(iPoly, NormalIds);
				if (status == MStatus::kFailure) {
					MGlobal::displayError("MFnMesh::getFaceNormalIds()");
					return status;
				}
			}

			// --- Loop over all face-vertices
			unsigned int iTriVertex;
			for (iTriVertex = 0; iTriVertex < 3; ++iTriVertex) {
				
				MeshFaceVertex FaceVertex;

				// Get polygon vertex id
				unsigned int iPolyVertex;
				iPolyVertex = MayaGeometry.TriangleVertexIds[3*iTri + iTriVertex];

				// Lookup and store face-vertex position
				MPoint mayaPoint;
				int iVertex = VertexIds[iPolyVertex];
				mayaPoint = MayaGeometry.Vertices[iVertex];
                MFloatArray& weights = MayaGeometry.Weights[iVertex];

				FaceVertex.vecPosition.x = mayaPoint.x;
				FaceVertex.vecPosition.y = mayaPoint.y;
				FaceVertex.vecPosition.z = mayaPoint.z;

				// Lookup and store face-vertex normal
				if (OPTIONS.exportNormals) {
					MVector mayaNormal;
					int iNormal = NormalIds[iPolyVertex];
					mayaNormal = MayaGeometry.FaceVertexNormals[iNormal];
					FaceVertex.vecNormal.x = mayaNormal.x;
					FaceVertex.vecNormal.y = mayaNormal.y;
					FaceVertex.vecNormal.z = mayaNormal.z;
				}

				// Lookup and store face-vertex colour
				if (OPTIONS.exportColours) {
					int iColour;
					MColor mayaColour;
					status = fnMesh.getFaceVertexColorIndex(iPoly, iPolyVertex, iColour);
					mayaColour = MayaGeometry.FaceVertexColours[iColour];
					FaceVertex.colour.r = mayaColour.r;
					FaceVertex.colour.g = mayaColour.g;
					FaceVertex.colour.b = mayaColour.b;
					FaceVertex.colour.a = mayaColour.a;
				}

				// Loop over UV sets
				if (OPTIONS.exportUVs) {
					MeshMayaUVSetList::iterator iterUVSet;
					iterUVSet = MayaGeometry.UVSets.begin();
					while (iterUVSet != MayaGeometry.UVSets.end()) {
						int iUV;
						fnMesh.getPolygonUVid(iPoly, iPolyVertex, iUV, &(iterUVSet->sName));
					
						MeshVertexUV VertexUV;
						VertexUV.u = iterUVSet->uArray[iUV];
						VertexUV.v = iterUVSet->vArray[iUV];
					
						FaceVertex.listUV.push_back(VertexUV);
		
						++iterUVSet;
					}
				}

                // assign weights
                {
                    const float eps = 0.001f;
                    float acc;                    

                    for(int i=weights.length()-1; i>=0; i--) {
                        acc = 0;
                        if(weights[i] < eps) {
                            acc += weights[i];
                        }
                        else {
                            VertexBoneAssigment vba;

                            vba.boneId   = i;
                            vba.vertexId = FaceVertices.size();
                            vba.weight   = weights[i];// + (acc/(float)i);

                            FaceVertex.boneAssigments.push_front(vba);
                        }
                    }                    
                }

				// Add FaceVertex to list
				FaceVertices.push_back(FaceVertex);
			}

			// --- Define face (three face-vertices)
			MeshTriFace TriFace;
			TriFace.index0 = (unsigned long)FaceVertices.size() - 3;
			TriFace.index1 = (unsigned long)FaceVertices.size() - 2;
			TriFace.index2 = (unsigned long)FaceVertices.size() - 1;

			TriFaces.push_back(TriFace);
		}

		return MStatus::kSuccess;
	}


	//	--------------------------------------------------------------------------
	/** Optimize mesh by removing redundant face-vertices. Relies on mMeshOptions
		member variable for settings.
	*/	
	//	--------------------------------------------------------------------------
    
	void MeshGenerator::_optimizeMesh(MeshFaceVertexVector &FaceVertices,
		                              MeshTriFaceList &TriFaces)
	{
        // TODO
        /*
		// ===== Optimize
		// Eliminate duplicate face-vertices
		//   - duplication is determined using position and normal tolerances
		//   - current implementation is very inefficient (O(n2))
		LogManager::getSingleton().logMessage(LML_NORMAL, "MeshGenerator: Attempting to optimize %d vertices...", FaceVertices.size());

		// Variables
		unsigned long iOuter, iInner;
		std::list<unsigned long> listRedundantIndices;
		MeshFaceVertexVector::iterator iterOuter, iterInner;
		std::list<MeshFaceVertexVector::iterator> listRedundantIters;

		// Loop over all face-vertices
		iOuter = 0;
		iterOuter = FaceVertices.begin();
		while (iterOuter != FaceVertices.end())
		{
			
			// Identify redundant (nearly equal) face-vertices
			listRedundantIndices.clear();
			listRedundantIters.clear();

			iInner = iOuter + 1;
			iterInner = iterOuter;
			++iterInner;
			while (iterInner != FaceVertices.end())
			{
				if (iterOuter->isAlmostEqual(&(FaceVertices[iInner]),
					                         mMeshOptions.bNormals, 
				                             mMeshOptions.bColours, 
				                             mMeshOptions.bUVs,
				                             mMeshOptions.fVertexPosTol, 
				                             mMeshOptions.fVertexNormTol)) 
				{
					listRedundantIndices.push_front(iInner);
					listRedundantIters.push_front(iterInner);
				}
				++iterInner;
				++iInner;
			}
			
			// Process redundant face-vertices
			if (listRedundantIndices.size() > 0) 
			{
				// Average vertex position and normal
				MeshFaceVertex FVMerged;
				FVMerged.vecNormal   = FaceVertices[iOuter].vecNormal;
				FVMerged.vecPosition = FaceVertices[iOuter].vecPosition;

				std::list<unsigned long>::iterator iterRedundantIndex;
				iterRedundantIndex = listRedundantIndices.begin();
				while (iterRedundantIndex != listRedundantIndices.end())
				{
					FVMerged.vecNormal   += FaceVertices[*iterRedundantIndex].vecNormal;
					FVMerged.vecPosition += FaceVertices[*iterRedundantIndex].vecPosition;
					++iterRedundantIndex;
				}
				FVMerged.vecPosition /= (listRedundantIndices.size() + 1);
				FVMerged.vecNormal.normalise();

				FaceVertices[iOuter].vecNormal   = FVMerged.vecNormal;
				FaceVertices[iOuter].vecPosition = FVMerged.vecPosition;
	
				// Remove redundant face-vertices
				std::list<MeshFaceVertexVector::iterator>::iterator iterRedundantIter;
				iterRedundantIter = listRedundantIters.begin();
				while (iterRedundantIter != listRedundantIters.end())
				{
					FaceVertices.erase(*iterRedundantIter);
					++iterRedundantIter;
				}

				// Replace references to redundant face-vertices
				MeshTriFaceList::iterator iterFaces;
				iterFaces = TriFaces.begin();
				while (iterFaces != TriFaces.end())
				{
					iterRedundantIndex = listRedundantIndices.begin();
					while (iterRedundantIndex != listRedundantIndices.end())
					{
						unsigned long iRedundant = *iterRedundantIndex;

						if (iterFaces->index0 == iRedundant) iterFaces->index0 = iOuter;
						if (iterFaces->index1 == iRedundant) iterFaces->index1 = iOuter;
						if (iterFaces->index2 == iRedundant) iterFaces->index2 = iOuter;

						if (iterFaces->index0 > iRedundant) --(iterFaces->index0);
						if (iterFaces->index1 > iRedundant) --(iterFaces->index1);
						if (iterFaces->index2 > iRedundant) --(iterFaces->index2);

						++iterRedundantIndex;
					}
					++iterFaces;
				}
			}


			// Increment outer loop
			++iterOuter;
			++iOuter;
		}

		LogManager::getSingleton().logMessage(LML_NORMAL, "MeshGenerator: Optimized to %d vertices", FaceVertices.size());
        */
	}
    

	//	--------------------------------------------------------------------------
	/** Determines if a given DAG node is currently visible.
		\param		fnDag
					DAG node to check
		\param		status
					Status code from Maya API

		\return		True if node is visible, False if node is not visible or if
					unable to determine visibility
	*/	
	//	--------------------------------------------------------------------------
	bool MeshGenerator::_isVisible(MFnDagNode &fnDag, MStatus &status) 
	{
		if(fnDag.isIntermediateObject()) {
			return false;
		}

		bool bVisible = false;
		MPlug visPlug = fnDag.findPlug("visibility", &status);
		if (MStatus::kFailure == status) {
			MGlobal::displayError("MPlug::findPlug");
		} else {
			status = visPlug.getValue(bVisible);
			if (MStatus::kFailure == status) {
				MGlobal::displayError("MPlug::getValue");
			}
		}

		return bVisible;
	}


	//	--------------------------------------------------------------------------
	/** Convert an array of object-relative vertex indices into an array of face-
		relative vertex indices. Required because MItMeshPolygon::getTriangle()
		returns object-relative vertex indices, whereas many other methods require
		face-relative vertex indices.

		Adapted from "How do I write a polygon mesh exporter?" at URL 
		http://www.ewertb.com/maya/api/api_a18.html.

		\param		iterPoly
					Reference to a polygon iterator that is currently at the 
					polygon of interest
		\param		objIndices
					Reference to array of object-relative indices
		\param		faceIndices
					Reference to array of face-relative indices (output). Indices
					are appended to the end of this array. A value of -1 will be 
					appended if there is no corresponding vertex.
	*/	
	//	--------------------------------------------------------------------------
	void MeshGenerator::_convertObjectToFace(MItMeshPolygon &iterPoly, 
		                                     MIntArray &objIndices, 
							                 MIntArray &faceIndices)
	{
		MIntArray polyIndices;
		iterPoly.getVertices(polyIndices);
			
		bool bMatched;
		unsigned int iPoly, iObj;
		for (iObj=0; iObj < objIndices.length(); ++iObj)
		{
			bMatched = false;
			
			// iPoly is face-relative vertex index
			for (iPoly=0; iPoly < polyIndices.length(); ++iPoly)
			{
				if (objIndices[iObj] == polyIndices[iPoly]) {
					faceIndices.append(iPoly);
					bMatched = true;
					break;
				}

			}

			// default if no match found
			if (!bMatched) {
				faceIndices.append(-1);
			}

		}
	}
	

} // namespace OgreMaya

#include "Grapher3D.h"

#if 0
struct Cell {
    uint8 Edges[4];
    uint8 AltEdges[4];   //Usually always 0
    bool Flag;          //When this flag is set, we need to calculate the midpoint of the box and choose whether to use edges or alt edges based on the center

};

static Cell mapCells[16] = {
/* 00 */    { {}, {}, false },
/* 01 */    { {2, 3}, {}, false },
/* 02 */    { {3, 4}, {}, false },
/* 03 */    { {2, 4}, {}, false },

/* 04 */    { {1, 2}, {}, false },
/* 05 */    { {1, 3}, {}, false },
/* 06 */    { {1, 2, 3, 4}, {1, 4, 2, 3}, true },
/* 07 */    { {1, 4}, {}, false },

/* 08 */    { {1, 4}, {}, false },
/* 09 */    { {1, 4, 2, 3}, {1, 2, 3, 4}, true },
/* 10 */    { {1, 3}, {}, false },
/* 11 */    { {1, 2}, {}, false },

/* 12 */    { {2, 4}, {}, false },
/* 13 */    { {3, 4}, {}, false },
/* 14 */    { {2, 3}, {}, false },
/* 15 */    { {}, {}, false },

};

void Grapher3D::Calculate(FuncType func) {
    Assert(func);

    int res = 4;    //resolution.. Divides the minor square into more tinier squares to sample from.. 
    double l = g_gridMinorL - g_gridMinorInc;
    double r = g_gridMinorR + g_gridMinorInc;
    double t = g_gridMinorT + g_gridMinorInc;
    double b = g_gridMinorB - g_gridMinorInc;

    double inc = g_gridMinorInc / res;
    int size = int( (r - l)/inc ) + 1;

//Visualise points
#if 0
    for (double y = b; y <= t; y += inc) {
        for (double x = l; x <= r; x += inc) {
            float off = +0.05;
            LineRenderer::DrawLine({x,y,0.0}, {x+off, y+off, 0.0}, 1, {1.0, 0.0, 0.0, 1.0});
        }
    }
    // LogWarn("%f ~ %f", g_gridL, g_gridR);
    LogWarn("size: %d", size);
    return;
#endif

    myLines.clear();
    myLines.reserve(1000);

    double* buff1 = new double[size];
    double* buff2 = new double[size];

    double* pBufOld = buff1;
    double* pBufCur = buff2;
    
    //Calculate the first row
    {
        int index = 0;
        for (double x = l; x <= r; x += inc) {
            pBufOld[index++] = func(x, b);
        }
    }

    //Iterate over row 2 onwards
    for (double y = b+inc; y <= t; y += inc) {
        pBufCur[0] = func(l, y);
        int index = 1;
        // LogWarn("00 (%.1f, %.1f): %.1f", l, y, pBufCur[0]);

        for (double x = l+inc; x <= r; x += inc) {
            Assert(index < size);
            pBufCur[index] = func(x, y);

            double cellVals[4] = { pBufOld[index-1], pBufOld[index], pBufCur[index], pBufCur[index-1] };

            uint8 iBL = (pBufOld[index-1] >= 0.0) ? 1 : 0; 
            uint8 iBR = (pBufOld[index]   >= 0.0) ? 1 : 0; 
            uint8 iTL = (pBufCur[index-1] >= 0.0) ? 1 : 0; 
            uint8 iTR = (pBufCur[index]   >= 0.0) ? 1 : 0;

            //visualise the graph.. Point is coloured red/green whether it is < 0 or > 0. The region in between the two is the plot of the graph
            #if 0
            {
                float off = 0.05;
                glm::vec4 col1 = {1.0, 0.0, 0.0, 1.0};
                glm::vec4 col2 = {0.0, 1.0, 0.0, 1.0};
                LineRenderer::DrawLine({x,y,0.0}, {x+off, y+off, 0.0}, 2.5, 
                    (iTR) ? col1 : col2);
                continue;
            }   
            // LogWarn("%02d (%.1f, %.1f): %.1f", index, x, y, pBufCur[index]);
            #endif

            int code = ( iBL << 3 | iBR << 2 | iTL << 1 | iTR );
            // LogWarn("%d", code);

            //Process code
            {
                Assert(code < 16);
                Cell* pc = &mapCells[code];
                uint8* pEdges;
                if (!pc->Flag)
                    pEdges = pc->Edges;
                else {
                    bool b = func(x - inc/2, y - inc/2) >= 0.0;
                    pEdges = b ? pc->AltEdges : pc->Edges; 
                }

                glm::vec2 positions[2];
                for (int line = 0; line < 4; line += 2) {
                    if (pEdges[line] == 0)
                        break;
                    for (int k = 0; k < 2; k++) {
                        uint8 edge = pEdges[line+k];
                        
                        // {
                        //     double targetX = 0.5;
                        //     double targetY = 0.0;
                        //     double givenX = x;
                        //     double givenY = y;

                        //     if (givenX < targetX+0.01 && givenX > targetX-0.01 && givenY < targetY+0.01 && givenY > targetY-0.01 || 0) {
                        //         LogWarn("EDge: %d", edge);
                        //     }

                        // }

                        switch(edge) {
                            //Note x and y are positions of the top right coordinate of a cell
                            case 1:
                            {
                                double p = Percent(cellVals[0], cellVals[1], 0.0);
                                positions[k] = { x-inc + inc*p, y-inc };
                                break;
                            }
                            case 3:
                            {
                                double p = Percent(cellVals[3], cellVals[2], 0.0);
                                positions[k] = { x-inc + inc*p, y };
                                break;
                            }

                            case 2:
                            {
                                double p = Percent(cellVals[1], cellVals[2], 0.0);
                                positions[k] = { x, y-inc + inc*p };
                                break;
                            }
                            
                            case 4:
                            {
                                double p = Percent(cellVals[0], cellVals[3], 0.0);
                                positions[k] = { x-inc, y-inc + inc*p };
                                break;
                            }
                        }
                        

                    }

                    // double targetX = 0.5;
                    // double targetY = 0.0;
                    // double givenX = positions[0].x;
                    // double givenY = positions[0].y;

                    // if (givenX < targetX+0.01 && givenX > targetX-0.01 && givenY < targetY+0.01 && givenY > targetY-0.01 || 1) {
                    //     // LogWarn("Code: %d", code);
                    //     // LogWarn("x: %f y: %f", x, y);
                        
                    // }

                    myLines.push_back({ positions[0], positions[1] });
                
                }
            }

            index++;
            
        } //End of second for loop iterating over x
        // LogWarn("");

        //Swap the buffers
        double* temp = pBufCur;
        pBufCur = pBufOld;
        pBufOld = temp;

        // break;  //Run only once for now
    }

    delete[] buff1;
    delete[] buff2;

}
#endif


void Grapher3D::Calculate(Renderer* r) {
    if (!myEquation)
        return;

    if (myEquation->IParamCount() < 3 ) {
        CalculateExplicit(r);
    }
    else {
        Assert("Unimplemented");
    }

}

void Grapher3D::CalculateExplicit(Renderer* r) {

    myStrips.clear();

    //Todo: make the bounds more dynamic.. Maybe based on the 
    const glm::vec2 boundX = { -10, 10 };
    const glm::vec2 boundY = { -10, 10 };
    const double incY = 0.25;
    const double incX = 0.25;

    std::vector<double> buffer1, buffer2;
    std::vector<double> *pbPrev = &buffer1, *pbCur = &buffer2;

    double eps = 0.001;
#if 0
    for (double y = boundY[0]; y < boundY[1] + eps; y += incY) {
        for (double x = boundX[0]; x < boundX[1] + eps; x += incX) {
            r->DrawPoint( glm::vec3(x, y, 0.01f), glm::vec4(1.0, 0.0, 0.0, 1.0), 1);
        }
    }
    return;
#endif

    for (double x = boundX[0]; x < boundX[1] + eps; x += incX) {
        const double y = boundY[0];
        
        // double z = func(x, y);
        Assert(myEquation);
        double z = myEquation->Evaluate(x, y);

        pbPrev->push_back(z);
    }

    //Skip the first row as we already processed it above
    double prevY = boundY[0];
    for (double y = boundY[0] + incY; y < boundY[1] + eps; y += incY) {
        int i = 0;
        TriangleStrip strip;

        for (double x = boundX[0]; x < boundX[1] + eps; x += incX) {
            // double z = func(x, y);
            Assert(myEquation);
            double z = myEquation->Evaluate(x, y);

            pbCur->push_back(z);

            Assert (i < pbPrev->size() && "Expected buffer size to match");
            if (i < pbPrev->size()) {
                double prevZ = pbPrev->at(i);
                strip.Positions.emplace_back( x, prevY, prevZ );
                strip.Positions.emplace_back( x, y, z );
                i++;
            }
        }
        myStrips.push_back(std::move(strip));

        prevY = y;
        std::swap(pbPrev, pbCur);
        pbCur->clear();
    }
}

void Grapher3D::CalculateImplicit(FuncImplicitType func) {

}

void Grapher3D::Draw(Renderer* r) {
    glm::vec4 col = {0.75, 0.75, 0.75, 1.0};

    bool bWireframe = 0;
    if (bWireframe)
        r->PushPolygonState(RE_POLYGON_LINE);

    r->PushDepthState(RE_DEPTH_LESS);
    for (const TriangleStrip& strip : myStrips) {
        r->DrawTriangleStrip(strip.Positions.data(), strip.Positions.size(), col);
    }
    r->PopDepthState();

    if (bWireframe)
        r->PopPolygonState();

}
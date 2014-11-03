#include "graph_view.h"

#include <assert.h>

#include <QtOpenGL>
#include <GLES2/gl2.h>

#define GL_CHECK() CheckError(__FILE__, __LINE__)

GraphViewRenderer* GraphView::m_renderer = NULL;

const char *
GraphViewRenderer::vshader = "attribute vec2 coord2d;\n" 
    "uniform float time_offset;\n"
    "void main(void) {\n" 
    "  float x = coord2d.x - time_offset;"
    "  gl_Position = vec4(x, coord2d.y, 0, 1);\n" 
    "}";

const char *
GraphViewRenderer::fshader = "void main(void) {"
    "  gl_FragColor = vec4(0,0,0,1);"
    "}";

void
GraphViewRenderer::AddMetric(int id)
{
    m_set = new GraphSet();
    m_subscriber->AddSet(id, m_set);
}

GraphViewRenderer::GraphViewRenderer(GraphSetSubscriber *s) : m_subscriber(s), 
                                                              m_set(NULL)
{

    // TODO remove hack
    m_set = new GraphSet();
    int id = m_subscriber->m_metrics[0]->met_id();
    m_subscriber->AddSet(id, m_set);


    glGenBuffers(1, &vbo);
    GL_CHECK();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GL_CHECK();

    const int vs = glCreateShader(GL_VERTEX_SHADER);
    GL_CHECK();
    int len = strlen(vshader);
    glShaderSource(vs, 1, &vshader, &len);
    GL_CHECK();
    glCompileShader(vs);
    PrintCompileError(vs);

    const int fs = glCreateShader(GL_FRAGMENT_SHADER);
    GL_CHECK();
    len = strlen(fshader);
    glShaderSource(fs, 1, &fshader, &len);
    GL_CHECK();
    glCompileShader(fs);
    PrintCompileError(fs);

    prog = glCreateProgram();
    glAttachShader(prog, vs);
    GL_CHECK();
    glAttachShader(prog, fs);
    GL_CHECK();
    glLinkProgram(prog);
    GL_CHECK();

    attribute_coord2d = glGetAttribLocation(prog,  "coord2d");
    GL_CHECK();
    uniform_time = glGetUniformLocation(prog, "time_offset");
    GL_CHECK();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GL_CHECK();
}

void 
GraphViewRenderer::render()
{
    if (! m_set)
    {
        update();
        return;
    }

    static GraphSet::PointVec data;
    m_set->GetData(&data);
    if (data.empty() )
    {
        update();
        return;
    }

    // static int last_size = 0;
    // if (last_size != data.size())
    // {
    //     std::cout << "size: " << data.size() << std::endl;
    //     for (int i = 0; i < data.size(); ++i )
    //     {
    //         std::cout << data[i].x << "," << data[i].y << std::endl;
    //     }
    // }
    // last_size = data.size();

    glClearColor(1,1,1,1);
    GL_CHECK();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GL_CHECK();
    glUseProgram(prog);
    GL_CHECK();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GL_CHECK();

    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GraphSet::Point), 
                 data.data(), GL_STATIC_DRAW);
    GL_CHECK();

    glEnableVertexAttribArray(attribute_coord2d);
    GL_CHECK();

    glVertexAttribPointer(attribute_coord2d,   // attribute
                          2,                   // number of elements per vertex, here (x,y)
                          GL_FLOAT,            // the type of each element
                          GL_FALSE,            // take our values as-is
                          0,                   // no space between values
                          0                    // use the vertex buffer object
        );
    GL_CHECK();

            
    // GLint t = GetTimeOffset();
    // assert(t >= 0);
    // assert(t <2000);
    // float x_offset = t / 1000.0;
    // glUniform1f(uniform_time, x_offset);
    glUniform1f(uniform_time, 0.0);
    GL_CHECK();

    glDrawArrays(GL_LINE_STRIP, 0, data.size());
    GL_CHECK();

    // x_offset = -2.0 + (t / 1000.0);
    // printf("t:%i\tx:%f\ty:%f\tcount:%d\n", t, graph[t].x - x_offset, graph[t].y, t);
    // glUniform1f(uniform_time, x_offset);
    // GL_CHECK();
    // glDrawArrays(GL_LINE_STRIP, 0, t);
    // GL_CHECK();
 
    glDisableVertexAttribArray(attribute_coord2d);
    GL_CHECK();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GL_CHECK();

    update();
}

void 
GraphViewRenderer::CheckError(const char * file, int line)
{
    if (glGetError() == GL_NO_ERROR)
        return;
    printf("ERROR: %s:%i\n");
    exit(-1);
}

void 
GraphViewRenderer::PrintCompileError(GLint shader)
{
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE)
        return;
    static const int MAXLEN = 1024;
    std::vector<char> log(MAXLEN);
    GLsizei len;
    glGetShaderInfoLog(shader,  MAXLEN,  &len, log.data()); 
    printf("ERROR -- compile failed: %s\n", log.data());
}

void 
GraphViewRenderer::synchronize(QQuickFramebufferObject * item) 
{
    GraphView *g = dynamic_cast<GraphView*>(item);
    if(g->m_id != -1)
    {
        AddMetric(g->m_id);
        g->m_id = -1;
    }
    if (g->m_pub)
    {
        AddPublisher(g->m_pub);
        g->m_pub = NULL;
    }
}

GraphView::GraphView() : m_subscriber(NULL), 
                         m_id(-1), m_pub(NULL) 
{
    setTextureFollowsItemSize(true);
}


// void 
// GraphView::SetSubscriber(GraphSetSubscriber *s) { m_subscriber = s; }
QQuickFramebufferObject::Renderer *
GraphView::createRenderer() const 
{ 
    assert (!m_renderer);
    m_renderer = new GraphViewRenderer(m_subscriber);
    if (m_id != -1)
        m_renderer->AddMetric(m_id);
    if (m_pub)
        m_renderer->AddPublisher(m_pub); 
    return m_renderer;
}

    
void 
GraphView::AddPublisher(Publisher* p) 
{ 
    m_pub = p;
}
void 
GraphView::AddMetric(int id) 
{ 
    m_id = id;
}
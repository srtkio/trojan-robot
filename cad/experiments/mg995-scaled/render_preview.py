"""Render out/assembly_standing.stl to out/preview_*.png (offscreen VTK)."""
import os
import vtk

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
reader = vtk.vtkSTLReader()
reader.SetFileName(os.path.join(out, "assembly_standing.stl"))
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(reader.GetOutputPort())
actor = vtk.vtkActor()
actor.SetMapper(mapper)
actor.GetProperty().SetColor(0.75, 0.8, 0.9)

ren = vtk.vtkRenderer()
ren.AddActor(actor)
ren.SetBackground(1, 1, 1)
win = vtk.vtkRenderWindow()
win.SetOffScreenRendering(1)
win.AddRenderer(ren)
win.SetSize(1400, 900)

views = {"iso": (1, -1.2, 0.7), "front": (1, 0, 0.05), "side": (0, -1, 0.05), "top": (0, 0, 1)}
for name, pos in views.items():
    cam = ren.GetActiveCamera()
    ren.ResetCamera()
    c = ren.GetActiveCamera().GetFocalPoint()
    cam.SetFocalPoint(*c)
    d = 900
    cam.SetPosition(c[0] + pos[0] * d, c[1] + pos[1] * d, c[2] + pos[2] * d)
    cam.SetViewUp(0, 0, 1) if name != "top" else cam.SetViewUp(0, 1, 0)
    ren.ResetCameraClippingRange()
    win.Render()
    w2i = vtk.vtkWindowToImageFilter()
    w2i.SetInput(win)
    w2i.Update()
    wr = vtk.vtkPNGWriter()
    wr.SetFileName(os.path.join(out, f"preview_{name}.png"))
    wr.SetInputConnection(w2i.GetOutputPort())
    wr.Write()
    print("wrote", name)

======================================================================

UC Berkeley Garment Library
---------------------------

This distribution contains geometric models of garments that are
intended for use with a cloth simulation.

** You are free to use these files provided that any publication,
** animation, or image produced through such use include an
** acknowledgment indicating that the clothing models were obtained
** from the UC Berkeley Garment Library.

The persons responsible for creating these files are:

    Juan Miguel de Joya
    Rahul Narain 
    James O'Brien 
    Armin Samii 
    Victor Zordan 


Example animations generated using a few of these garments may be 
found at http://graphics.berkeley.edu/papers/Narain-AAR-2012-11


======================================================================

Garments:

Each garment is provided both as a coarse base mesh and a finer
isotropically refined mesh.  The coarse base meshes are suitable for
use with a simulation engine that supports dynamic mesh refinement,
such as ARCSim. (http://graphics.berkeley.edu/resources/ARCSim) The
refined meshes are suitable for other simulators that use a fixed mesh
topology.  If desired, the refined meshes may be further refined to
higher resolution using linear subdivision.

Each mesh is represented using three files stored in .obj format.  The
files with a -m suffix contain the meshes in their material (aka
reference) configuration.  The files with a -w suffix contain the
meshes in their world-space configuration.  The files without a -m or
-w suffix contain a combined representation in world coordinates where
seam vertices have been merged and texture coordinates are used to
store the material coordinates for each vertex.

Important: The world-space files contain configurations where the 
garment has been stretched around the figure, often in a highly 
distorted fashion.  The material-space reference meshes determine the
correct rest size and shape for each triangle in the mesh.

A few of the meshes are also provided in a configuration where the
garment is already draped over the figure in a physically plausible
configuration.

======================================================================

Materials and Motion:

Additional files in the Materials and Motion directories are provided
for convenience.  Please see their respective README files.

======================================================================


(C) Copyright UC Berkeley

These files are provided "as is" and any express or implied
warranties, including, but not limited to, the implied warranties of
merchantability and fitness for a particular purpose are
disclaimed. In no event shall the copyright holder be liable for any
direct, indirect, incidental, special, exemplary, or consequential
damages (including, but not limited to, procurement of substitute
goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether
in contract, strict liability, or tort (including negligence or
otherwise) arising in any way out of the use of these files, even if
advised of the possibility of such damage.

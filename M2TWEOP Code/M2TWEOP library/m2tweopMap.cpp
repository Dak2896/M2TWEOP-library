#include "m2tweopMap.h"
#include <imgui_internal.h>

void m2tweopMap::buildMap()
{
	tiles.clear();
	borders.clear();

	gameDataAllStruct* gameDataAll = reinterpret_cast<gameDataAllStruct*>(dataOffsets::offsets.gameDataAllOffset);
	tilesS* tilesMap = gameDataAll->stratMap;
	xBound = tilesMap->xBound;
	yBound = tilesMap->yBound;

	for (int x = 0; x < xBound; x++)
	{
		for (int y = 0; y < yBound; y++)
		{
			shared_ptr< m2tweopTile >newTile = std::shared_ptr<m2tweopTile>(new m2tweopTile);

			newTile->buildTile(x, y);


			tiles.push_back(std::move(newTile));
		}
	}


	for (int x = 1; x < xBound - 1; x++)
	{
		for (int y = 1; y < yBound - 1; y++)
		{
			auto currTile = tiles[x * yBound + y];

			auto leftTile = tiles[(x - 1) * yBound + y];
			auto rightTile = tiles[(x + 1) * yBound + y];
			auto upTile = tiles[x * yBound + y + 1];
			auto lowTile = tiles[x * yBound + y - 1];
			currTile->buildBorder(
				tiles[(x - 1) * yBound + y],
				tiles[(x + 1) * yBound + y],
				tiles[x * yBound + y + 1],
				tiles[x * yBound + y - 1]
			);
		}
	}
}

void m2tweopMap::drawMap(const ImVec2& tileSize)
{
	borders.clear();


	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0,0 });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 mapStartPos = ImGui::GetWindowPos();
	mapStartPos.x -= ImGui::GetScrollX();
	mapStartPos.y -= ImGui::GetScrollY();


	for (int y = yBound - 1; y >= 0; y--)
	{
		for (int x = 0; x < xBound; x++)
		{
			auto& currTile = tiles[x * yBound + y];
			bool isSelected = (selectedTile!=nullptr&&selectedTile->IsSameCoords(currTile));
			if (true == currTile->drawTile(tileSize, mapStartPos, &borders, isSelected))
			{
				selectedTile = tiles[x * yBound + y];
			}

			ImGui::SameLine();
		}

		ImGui::Dummy(tileSize);

	}

	if(needDrawBorders==true)
	{ 
		for (size_t i = 0; i < borders.size()/2; i++)
		{
			drawList->AddLine(borders[i*2], borders[i*2+1], IM_COL32(200, 200, 200, 255), 0.2f * tileSize.y);
		}
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void m2tweopMap::drawInteract()
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanAvailWidth;
	if (ImGui::TreeNodeEx("Map settings", node_flags) == true)
	{
		ImGui::Checkbox("Draw borders", &needDrawBorders);


		ImGui::TreePop();
	}
	if (ImGui::TreeNodeEx("Selected tile", node_flags) == true)
	{
		if (selectedTile != nullptr)
		{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

				ImGui::InputScalar("X coordinate", ImGuiDataType_U32, &selectedTile->xTile, 0, NULL, "%u");
				ImGui::InputScalar("Y coordinate", ImGuiDataType_U32, &selectedTile->yTile, 0, NULL, "%u");
				ImGui::InputScalar("Region ID", ImGuiDataType_U32, &selectedTile->tileRegionID, 0, NULL, "%u");

				ImGui::Separator();

				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
		}




		ImGui::TreePop();
	}

}

